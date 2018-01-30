# nightscout-ps1
[![CircleCI](https://circleci.com/gh/TooTallNate/nightscout-ps1/tree/master.svg?style=svg)](https://circleci.com/gh/TooTallNate/nightscout-ps1/tree/master)

<img width="245" src="https://user-images.githubusercontent.com/71256/34074189-a4a58b6e-e25e-11e7-8368-b12e684fdd04.png">

Tiny C program that formats your latest Nightscout BGL reading for use in
your terminal prompt (a.k.a $PS1).

The trend and target high/low values are also provided, so that you may
render arrows and colors as desired:

<img width="77" src="https://user-images.githubusercontent.com/71256/34065696-98696f46-e1b9-11e7-9e7e-b59386fc8bcf.png">

### Installation

Preferred installation is by downloading a pre-compiled binary for your platform:

* [GitHub Releases](https://github.com/TooTallNate/nightscout-ps1/releases)

If there is no binary for your platform, or you would simply like to install
from source, invoke the `make` command:

```bash
$ make
$ ./out/Default/nightscout-ps1
```

### Configure your `$PS1`

Add this to your `.bashrc` file:

```bash
export PS1="\$(nightscout-ps1) $ "
```

Be sure to add further customizations to your `$PS1` from there!
