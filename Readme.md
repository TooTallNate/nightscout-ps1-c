# nightscout-ps1

[![CircleCI](https://circleci.com/gh/TooTallNate/nightscout-ps1/tree/master.svg?style=svg)](https://circleci.com/gh/TooTallNate/nightscout-ps1/tree/master)
[![AppVeyor](https://ci.appveyor.com/api/projects/status/1rp3biuu82e76fgq/branch/master?svg=true)](https://ci.appveyor.com/project/TooTallNate/nightscout-ps1/branch/master)

<img width="245" src="https://user-images.githubusercontent.com/71256/34074189-a4a58b6e-e25e-11e7-8368-b12e684fdd04.png">

Tiny C program that formats your latest Nightscout BGL reading for use in
your terminal prompt (a.k.a $PS1). It's written in C to be as fast as possible,
since it gets executed every time the prompt is rendered, so latency is critical.

You will need to set up [`nightscout-ps1-daemon`][daemon] which is responsible for
connecting to your Nightscout server and creating the INI file that this
program reads.

The trend and target high/low values are also provided, so that you may
render arrows and colors as desired:

<img width="77" src="https://user-images.githubusercontent.com/71256/34065696-98696f46-e1b9-11e7-9e7e-b59386fc8bcf.png">

### Installation

Preferred installation is by downloading a pre-compiled binary for your platform:

* [GitHub Releases](https://github.com/TooTallNate/nightscout-ps1/releases)

If there is no binary for your platform, or you would simply like to install
from source, invoke the `make` command:

```bash
# build the `nightscout-ps1` binary
$ make
$ ./out/Default/nightscout-ps1

# install to /usr/local by default, set PREFIX for a custom dir
$ make install
```

### Configure your `$PS1`

Add this to your `.bashrc` file:

```bash
export PS1="\$(nightscout-ps1) $ "
```

Be sure to add further customizations to your `$PS1` from there!

[daemon]: https://github.com/TooTallNate/nightscout-ps1-daemon
