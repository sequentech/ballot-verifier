## agora-audit

agora-audit is a tool that allows you to audit a spoiled ballot from Agora Voting. This software has been tested on linux 64 bits. If you move the agora-audit executable, remember to move the screen.png to the same folder as the executable.

The agora-audit tool has a textbox on the upper left side where you should copy the ballot. Once you cast your vote in Agora Voting, you are allowed to audit the ballot (this also discards the ballot). The upper right side of agora-audit shows you a screen capture of the audit ballot webpage and marks the place where you will find the ballot with a red box. 

Once you have copied the ballot to agora-audit, you should click the "Verify Ballot" button. If the ballot is verified, the state indicator should change to State: VERIFIED. There is a console below the Details label that shows more info.

## Compilation

Go to the agora-airgap/src folder and execute:

    make gui
    
If the build is successful, you will find the agora-audit tool on agora-airgap/src/x64/agora-audit . Don't forget to copy along the screen.png file if you choose to move agora-audit somewhere else.