import paramiko
import csv
import matplotlib.pyplot as plt
from datetime import datetime
from io import StringIO

def read_remote_csv_with_ssh_agent(hostname, port, username, remote_filepath):
    """
    Reads a CSV file from a remote server via SSH using ssh-agent.

    Args:
        hostname (str): The hostname or IP address of the remote server.
        port (int): The SSH port number (usually 22).
        username (str): The username for SSH authentication.
        remote_filepath (str): The absolute path to the CSV file on the remote server.

    Returns:
        list: A list of rows from the CSV file, or None if an error occurs.
    """
    try:
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        # Use the ssh-agent for authentication
        agent = paramiko.Agent()

        client.connect(hostname=hostname, port=port, username=username)

        sftp = client.open_sftp()
        with sftp.open(remote_filepath, 'r') as remote_file:
            file_content = remote_file.read().decode('utf-8')
            csv_reader = csv.reader(StringIO(file_content))
            header = next(csv_reader) # Skip header
            data = list(csv_reader)
        client.close()
        return data

    except paramiko.AuthenticationException:
        print("Authentication failed, please verify your SSH agent is running and has the key loaded.")
        return None
    except paramiko.SSHException as ssh_exception:
        print(f"Could not establish SSH connection: {ssh_exception}")
        return None
    except FileNotFoundError:
        print(f"Remote file not found: {remote_filepath}")
        return None
    except Exception as e:
        print(f"An error occurred: {e}")
        return None

def plot_resource_usage_from_remote_ssh_agent(hostname, port, username, remote_filepath):
    """
    Reads CPU and memory usage from a remote CSV file via SSH using ssh-agent and plots it.

    Args:
        hostname (str): The hostname or IP address of the remote server.
        port (int): The SSH port number (usually 22).
        username (str): The username for SSH authentication.
        remote_filepath (str): The absolute path to the CSV file on the remote server.
    """
    data = read_remote_csv_with_ssh_agent(hostname, port, username, remote_filepath)

    if data:
        timestamps = []
        cpu_usages = []
        memory_usages = []

        for row in data:
            if len(row) == 3:
                timestamp_str, cpu_str, memory_str = row
                try:
                    timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S')
                    cpu_usage = float(cpu_str)
                    memory_usage = float(memory_str)
                    timestamps.append(timestamp)
                    cpu_usages.append(cpu_usage)
                    memory_usages.append(memory_usage)
                except ValueError:
                    print(f"Skipping invalid row: {row}")
            else:
                print(f"Skipping row with incorrect number of columns: {row}")

        if timestamps:
            plt.figure(figsize=(12, 6))
            plt.plot(timestamps, cpu_usages, label='CPU Usage (%)', color='blue')
            plt.plot(timestamps, memory_usages, label='Memory Usage (%)', color='red')
            plt.xlabel('Timestamp')
            plt.ylabel('Usage (%)')
            plt.title('CPU and Memory Usage Over Time (Remote)')
            plt.grid(True)
            plt.legend()
            plt.xticks(rotation=45, ha='right')
            plt.tight_layout()
            plt.show()
        else:
            print("No valid data to plot from the remote file.")
    else:
        print("Failed to read data from the remote server.")

# Your server details and remote file path
remote_host = '192.168.56.13'
remote_port = 22
remote_user = 'vagrant'
remote_file = '/var/log/resource_manager/system_monitor.csv'

plot_resource_usage_from_remote_ssh_agent(remote_host, remote_port, remote_user, remote_file)