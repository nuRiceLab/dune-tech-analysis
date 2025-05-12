# Persistent ROOT Environment Setup for JupyterHub (TLJH)

This repository provides a shell script to create a **persistent Conda environment** with the [CERN ROOT](https://root.cern) framework, designed for use in environments like **The Littlest JupyterHub (TLJH)**.

The environment is created under your home directory so it **survives server restarts**, and it is automatically registered as a Jupyter kernel for convenient use in notebooks.

---

## 📜 What the Script Does

The script `run_install.sh` performs the following steps:

1. **Sets Conda Channel Priority to Strict**  
   Ensures Conda prefers `conda-forge` packages to avoid mixing incompatible packages:
   ```bash
   conda config --set channel_priority strict
