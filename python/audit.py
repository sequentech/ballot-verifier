#!/usr/bin/env python3


from pprint import pprint

import urllib.request as req
import json

from Crypto.PublicKey import ElGamal
from Crypto.Random import random


def parse_ballot(path):
    ballot = None
    with open(path, 'r') as f:
        ballot = json.load(f)
    return ballot


def download_election(url):
    u = req.urlopen(url)
    data = json.loads(u.read().decode('utf-8'))
    return data


def download_pk(url):
    el = download_election(url)
    return json.loads(el['payload']['pks'])


def plain_encode(m, pk):
    # need to encode the message given that p = 2q+1
    n = m
    y = m + 1

    test = pow(y, pk.q, pk.p)

    if test == 1:
        n = y
    else:
        n = -y % pk.p

    return n


def getpk(key):
    tup = tuple(map(int, (key['p'], key['g'], key['y'])))
    pk = ElGamal.construct(tup)
    pk.q = int(key['q'])

    return pk


def test_encryption(choice, key):
    r = int(choice['randomness'])
    pk = getpk(key)

    m = plain_encode(int(choice['plaintext']), pk)

    a, b = pk.encrypt(m, r)

    assert(a == int(choice['alpha']))
    assert(b == int(choice['beta']))

    print('OK')


def custom_encryption(choice, key):
    pk = getpk(key)
    m = plain_encode(int(choice['plaintext']), pk)
    r = random.StrongRandom().randint(1, pk.p - 1)
    return pk.encrypt(m, r)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Audits agora election vote encryption')
    parser.add_argument('ballot', help='the ballot.json to audit')

    args = parser.parse_args()

    ballot = parse_ballot(args.ballot)
    pks = download_pk(ballot['election_url'])

    for c, k in zip(ballot["choices"], pks):
        test_encryption(c, k)
