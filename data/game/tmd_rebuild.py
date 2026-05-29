import os
import subprocess

def process_subdirectories(base_path):
    # Check if the given base path exists
    if not os.path.exists(base_path):
        print("The specified path does not exist.")
        return
    
    # Iterate through each subdirectory
    for root, dirs, files in os.walk(base_path):
        for directory in dirs:
            subdir_path = os.path.join(root, directory)
            
            # Delete all .tmd files in the subdirectory
            for file in os.listdir(subdir_path):
                if file.lower().endswith(".tmd"):
                    tmd_file_path = os.path.join(subdir_path, file)
                    try:
                        os.remove(tmd_file_path)
                    except Exception as e:
                        print("Error deleting", tmd_file_path, "-", e)
            
            # Call the CONV2.BAT file
            bat_file_path = os.path.join(subdir_path, "CONV2.BAT")
            if os.path.exists(bat_file_path):
                try:
                    subprocess.call([bat_file_path], cwd=subdir_path, shell=True)
                    print("Executed:", bat_file_path)
                except Exception as e:
                    print("Error executing", bat_file_path, "-", e)

if __name__ == "__main__":
    base_directory = r"C:\\code\\car-basic-working\\DATA\\track"
    process_subdirectories(base_directory)
