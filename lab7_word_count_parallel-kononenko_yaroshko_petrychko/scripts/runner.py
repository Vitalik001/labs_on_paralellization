# Parse the configuration file
import subprocess
config_file = './configs/config1.cfg'  # Replace with the actual filename
config_data = []
with open(config_file, 'r') as file:
    config_data = file.readlines()


# Store the configuration values
settings = {}
# print(config_data)
for i, line in enumerate(config_data):
    line = line.strip()
    if line and not line.startswith('#'):
        key, value = line.split('=')
        settings[key.strip()] = value.strip()
        if key.strip() == 'indexing_threads':
            indexing_line = i
        elif key.strip() == 'merging_threads':
            merging_line = i

#
# Modify the values and update the configuration file
for i in range(1, 10):
    indexing_threads = i
    merging_threads = i
    config_data[indexing_line] = f"indexing_threads = {indexing_threads}\n"
    config_data[merging_line] = f"merging_threads = {merging_threads}\n"
    with open(config_file, 'w') as file:
        file.writelines(config_data)
    # Run your script here
    subprocess.run(['python3', '/Users/vitalii/Desktop/UCU/AKS/lab7_word_count_parallel-kononenko_yaroshko_petrychko/scripts/script.py', '1', '0'])
#
