// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <agora-airgap/encrypt.h>
#include <agora-airgap/sha256.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace rapidjson;
using namespace std;
using namespace AgoraAirgap;

bool check_file_exists(string file_path)
{
    ifstream sfile(file_path.c_str());
    if (sfile.good())
    {
        sfile.close();
        return true;
    } else
    {
        sfile.close();
        return false;
    }
    return true;
}

void show_help()
{
    cout << "Usage: agora-airgap [option] file..." << endl
         << "Options:" << endl
         << "\tdownload-audit\t\t\tdownload election data and audit the ballot"
         << endl
         << "\tdownload\t\t\tdownload election data" << endl
         << "\taudit\t\t\t\taudit the ballot, providing the election data file"
         << endl
         << "\tencrypt\t\t\t\tencrypt the plaintext ballot, providing the "
            "public "
            "keys and the plaintext ballot files"
         << endl;
}

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        cout << "You need to supply more arguments. Example: " << argv[0]
             << " download-audit <file_with_auditable_ballot.json>" << endl;
        show_help();
        exit(1);
    }

    vector<string> vargs;

    for (int i = 0; i < argc; ++i)
    {
        vargs.push_back(string(argv[i]));
    }

    if (vargs.at(1) == string("download-audit"))
    {
        if (vargs.size() < 3)
        {
            cout << "You need to supply more arguments. Example: "
                 << vargs.at(0)
                 << " download-audit <file_with_auditable_ballot.json>" << endl;
            exit(1);
        } else if (vargs.size() > 3)
        {
            cout << "Error: too many arguments. " << endl;
            exit(1);
        } else if (!check_file_exists(vargs.at(2)))
        {
            cout << "Error: ballot file not found at path " << vargs.at(2)
                 << endl;
            exit(1);
        } else
        {
            try
            {
                stringstream out;
                download_audit(out, vargs.at(2));
                cout << out.str() << endl;
            } catch (std::runtime_error & error)
            {
                cout << error.what() << endl;
            }
        }
    } else if (vargs.at(1) == string("download"))
    {
        if (vargs.size() < 4)
        {
            cout << "You need to supply more arguments. Example: "
                 << vargs.at(0)
                 << " download <file_with_auditable_ballot.json> "
                    "<election_data_file>"
                 << endl;
            exit(1);
        } else if (vargs.size() > 4)
        {
            cout << "Error: too many arguments " << endl;
            exit(1);
        } else if (!check_file_exists(vargs.at(2)))
        {
            cout << "Error: ballot file not found at path " << vargs.at(2)
                 << endl;
            exit(1);
        } else if (check_file_exists(vargs.at(3)))
        {
            cout << "Error: cannot create election data file at path "
                 << vargs.at(3) << " as it already exists" << endl;
            exit(1);
        } else
        {
            try
            {
                stringstream out;
                download(out, vargs.at(2), vargs.at(3));
                cout << out.str() << endl;
            } catch (std::runtime_error & error)
            {
                cout << error.what() << endl;
            }
        }
    } else if (vargs.at(1) == string("audit"))
    {
        if (vargs.size() < 4)
        {
            cout << "You need to supply more arguments. Example: "
                 << vargs.at(0)
                 << " audit <file_with_auditable_ballot.json> "
                    "<election_data_file>"
                 << endl;
            exit(1);
        } else if (vargs.size() > 4)
        {
            cout << "Error: too many arguments. " << endl;
            exit(1);
        } else if (!check_file_exists(vargs.at(2)))
        {
            cout << "Error: ballot file not found at path " << vargs.at(2)
                 << endl;
            exit(1);
        } else if (!check_file_exists(vargs.at(3)))
        {
            cout << "Error: election data file not found at path "
                 << vargs.at(3) << endl;
            exit(1);
        } else
        {
            try
            {
                stringstream out;
                audit(out, vargs.at(2), vargs.at(3));
                cout << out.str() << endl;
            } catch (std::runtime_error & error)
            {
                cout << error.what() << endl;
            }
        }
    } else if (vargs.at(1) == string("encrypt"))
    {
        if (vargs.size() < 5)
        {
            cout << "You need to supply more arguments. Example: "
                 << vargs.at(0)
                 << " encrypt <file_with_plaintext_ballot.json> "
                    "<public_key_file> "
                    "<encrypted_ballot_file>"
                 << endl;
            exit(1);
        } else if (vargs.size() > 5)
        {
            cout << "Error: too many arguments. " << endl;
            exit(1);
        } else if (!check_file_exists(vargs.at(2)))
        {
            cout << "Error: plaintext ballot file not found at path "
                 << vargs.at(2) << endl;
            exit(1);
        } else if (!check_file_exists(vargs.at(3)))
        {
            cout << "Error: public key file not found at path " << vargs.at(3)
                 << endl;
            exit(1);
        } else if (check_file_exists(vargs.at(4)))
        {
            cout << "Error: cannot create encrypted ballot at path "
                 << vargs.at(4) << " as it already exists" << endl;
            exit(1);
        } else
        {
            try
            {
                stringstream out;
                encrypt_ballot(out, vargs.at(2), vargs.at(3), vargs.at(4));
                cout << out.str() << endl;
            } catch (std::runtime_error & error)
            {
                cout << error.what() << endl;
            }
        }
    } else
    {
        show_help();
    }
    return 0;
}