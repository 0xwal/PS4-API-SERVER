# PS4API Server

This project allows you to control PS4 game memory for reading/writing calls.
with this project you can make RTE/RTM tools


## Installation

use [CTurt's PS4 SDK from xvortex's repository](https://github.com/xvortex/ps4-payload-sdk).
then follow the instructions on how to add the sdk path to your environment.

## Usage

char sendCommand(char command, void* args);

commands:

    * 'a' attach to game process ( eboot.bin ) and the process will continued.
    * 'c' continue the process.
    * 'd' detach the process.
    * 's' suspend the process.
    * 'u' resume the process.
    * 'k' kill the process (seems not works)
    * 'n' send notification with text.
    * 'r' read memory.
    * 'w' write memory.

args:

    * 'a'(void);
    * 'c'(void);
    * 'd'(void);
    * 's'(void);
    * 'u'(void);
    * 'k'(void);
    * 'n'(char text[]);
    * 'r'(unsigned int length, unsigned long int address);
    * 'w'(unsigned int length, unsigned long int address, char data[]);    




## Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request :D


## Credits

CTurt -> the SDK.
Specter -> exploit implementation.
IDC, xvortex, *2much4u*, who else I missed (message me for the Credits)
