# building on mac:
some dependencies are needed:

* bash (coinbrew)
* wget (coinbrew)
* gcc (includes gfortran)

these can be obtained with `homebrew`:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

```bash
brew install bash wget gcc
```

# download, build and install CLP

to download and build CLP, first download `coinbrew`, if not already present:

```bash
wget https://raw.githubusercontent.com/coin-or/coinbrew/master/coinbrew
chmod +x coinbrew
```

See <https://coin-or.github.io/coinbrew/> for additional requirements and install only if necessary (i.e., coinbrew fails).

Then build and install CBC (prefix e.g., `(win64|linux64|osx)/dist`)

```bash
export ADD_FFLAGS=-fallow-argument-mismatch # only use for gfortran10 (from gcc10), when getting errors
export ADD_CXXFLAGS=-lrt # only use if getting the error: "undefined reference to `clock_gettime'"
./coinbrew build install Cbc@releases/2.10.8 --parallel-jobs $(nproc) --tests none --prefix $prefix
```
