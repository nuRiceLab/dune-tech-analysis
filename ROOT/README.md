# Persistent ROOT environment setup for your jupytherhub server

This repository provides a shell script to create a **persistent Conda environment** with the [CERN ROOT](https://root.cern) framework

The environment is created under your home directory so it **survives server restarts**, and it is automatically registered as a Jupyter kernel for convenient use in notebooks.

---

## What the script does

The script `run_install.sh` performs the following steps:

1. **Sets Conda Channel Priority to Strict**  
   Ensures Conda prefers `conda-forge` packages to avoid mixing incompatible packages:
   ```bash
   conda config --set channel_priority strict

2. **Create a Persistent Conda Environment with ROOT**
   ```bash
   conda create -y -c conda-forge --prefix $HOME/.conda/envs/root-env ROOT

3. **Activate the Environment and Register It as a Jupyter Kernel**
   ```bash
   conda activate $HOME/.conda/envs/root-env
   python -m ipykernel install --user --name root-env --display-name "ROOT"

## To run the script open a terminal and run the following command 
   ```bash 
   source run_install.sh

