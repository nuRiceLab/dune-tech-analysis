# 1. Ensure strict channel priority
conda config --set channel_priority strict

# 2. Create the environment in a persistent location (e.g., ~/.conda/envs/root-env)
conda create -y -c conda-forge --prefix $HOME/.conda/envs/root-env ROOT

conda activate $HOME/.conda/envs/root-env
python -m ipykernel install --user --name root-env --display-name "ROOT"
