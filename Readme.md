# Trivent - SDHCAL timing event builder

## Dependencies

You need cmake>3.5, gcc>5.0 (for c++14 features) and python>2.7
From ilcsoft you also need, Marlin, lcio, ilcutil, Root6

For ease of use you can use the clicdp installation of ilcsoft available on cvmfs at  `/cvmfs/clicdp.cern.ch/iLCSoft/builds/current/CI_gcc`
If for whatever reason this build doesn't work anymore you can use `/cvmfs/clicdp.cern.ch/iLCSoft/builds/2018-10-11/x86_64-slc6-gcc62-opt/`.

## Installation

```bash
    export ilcsoftScript=/cvmfs/clicdp.cern.ch/iLCSoft/builds/current/CI_gcc/init_ilcsoft.sh  # adapt to your needs
    git clone https://github.com/apingault/Trivent
    source $ilcsoftScript
    mkdir build; cd build; cmake -C $ILCSOFT/ILCSoft.cmake ..
    make install
```

If you want to use the python package to configure the processor:

```bash
    python -m python/setup install
```

I would strongly recommend the use of a virtualenv to ensure a properly working python environment

## Configuration
Edit the various paths to data/ilcsoft script etc.


## Running

To ensure proper environment I would suggest using a virtualenv: (These steps are needed only the first time around, it should take ~2min on lxplus)

``` bash
    # From the main dir
    source $ilcsoftScript
    curl -LO https://bootstrap.pypa.io/get-pip.py
    python get-pip.py --user
    export PY_USER_BIN=$(python -c 'import site; print(site.USER_BASE + "/bin")')
    export PATH=$PY_USER_BIN:$PATH
    python -m pip install --user virtualenv
    # python -m pip install --user --upgrade virtualenvwrapper
    # source virtualenvwrapper.sh
    # mkvirtualenv --python=$(which python) triventEnv
    virtualenv --no-site-packages -p $(which python) triventEnv  # Ensure you use the correct Python version not the ols system one(2.6.6)
    source triventEnv/bin/activate
    # Update python installation
    pip install pyyaml pymysql
```

Once this is set-up you can just:

```bash
    source triventEnv/bin/activate  # Use the virtual env
    python -m python/pyMarlin.py config/yourConfigFile  # without the .py extension at the end of the configFile
```

Once you are done you can leave the virtualenv by calling `deactivate`

