# Contributing

Thank you for your interest in LiteFX. Your contribution is most welcome, no matter comprehensive. However, please note the following guidelines before you submit a pull request:

- Don't fix multiple aspects in one PR. Instead focus on fixing one bug, adding one feature, updating one part of the documentation and so on... you get the idea. I know that this might not always be possible, however, just try to keep the PR as minimal as possible. 🙂
- When submitting your PR describe (as good as you can), what you have changed and why you've chosen the implemented approach. If applicable, of course!
- Don't commit changes to the following files and directories:
  - `./docs/docs/` - This directory contains the [documentation website](https://litefx.crudolph.io/docs/) and is automatically updated when a new release is published.
  - `./.doxyfile` - This file contains the Doxygen definitions. If you want to build your own documentation, copy it and use this version. If you *really* want to change this file, please also update the *release.yml* workflow as well!
  - `./src/cmake/VersionVariables.cmake` - This file is also automatically generated when a new release is published.

Note, however, that I am currently maintaining this repository in my free time and I may have only limited resources to check your PR. I try my best to give this project the care it deserves, so don't hesitate submitting your changes. 🚀