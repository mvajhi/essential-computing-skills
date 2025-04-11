#!/usr/bin/python3
import psutil
import time
import csv
import os
import subprocess
import json

last_message_time = 0

COOLDOWN = 30
THRESHOLD_CPU = 80
THRESHOLD_MEMORY = 80
PATH = "/var/log/resource_manager"
SUSPEND_LOG_PATH = PATH + "/suspend_log.csv"
USAGE_LOG_PATH = PATH + "/usage.csv"
RUN_INTERVAL = 1

CONFIG_PATH = "/etc/resource_manager/config.json"

def main():
    while True:
        cpu_usage, memory_usage = get_status()
        store_usage(cpu_usage, memory_usage)
        
        print(f"CPU Usage: {cpu_usage}%")
        print(f"Memory Usage: {memory_usage}%")
        
        if (has_crossed_threshold(cpu_usage, memory_usage)):
            send_message(cpu_usage, memory_usage)
            suspend_processes(cpu_usage, memory_usage)

        time.sleep(RUN_INTERVAL) 

def suspend_processes(cpu_usage, memory_usage): 
    process = get_highest_process('cpu' if has_crossed_threshold_cpu(cpu_usage) else 'mem')
    status = stop_process(process, has_crossed_threshold_mem(memory_usage))
    store_suspend_log(process, status, has_crossed_threshold_mem(memory_usage))

def stop_process(process, kill):
    try:
        if kill:
            process.kill()
        else:
            process.suspend()
        return True
    except (psutil.NoSuchProcess, psutil.AccessDenied):
        return False   

def store_suspend_log(process, status, kill):
    with open(SUSPEND_LOG_PATH, mode='a', newline='') as file:
        writer = csv.writer(file)
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        writer.writerow([timestamp, process.info['pid'], process.info['name'], status, kill])
    
    print(f"Suspended process: {process.info['name']} (PID: {process.info['pid']}) (Success: {status}) (Kill: {kill})")

def get_highest_process(criteria):
    proc_max = None
    criteria = 'cpu_percent' if criteria == 'cpu' else 'memory_percent'
    
    processes = psutil.process_iter(attrs=['pid', 'name', 'status', criteria])
    for process in processes:
        try:
            if process.info['status'] == psutil.STATUS_STOPPED or process.info['pid'] == os.getpid() or process.info['pid'] < 500:
                continue
            
            proc_max = process if proc_max is None else proc_max
            if process.info[criteria] > proc_max.info[criteria]:
                proc_max = process
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
    
    return proc_max
    
def send_message(cpu_usage, memory_usage):
    global last_message_time
    current_time = time.time()
    
    if current_time - last_message_time < COOLDOWN:
        return
    
    last_message_time = current_time 
    
    send_wall_message(f"Resource usage threshold crossed!\n\tCPU: {cpu_usage}%\n\tMemory: {memory_usage}%")
    

def send_wall_message(message):
    try:
        subprocess.run(["sudo","wall", message], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error sending wall message: {e}")

def has_crossed_threshold(cpu_usage, memory_usage) -> bool:
    return has_crossed_threshold_cpu(cpu_usage) or has_crossed_threshold_mem(memory_usage)

def has_crossed_threshold_cpu(cpu_usage) -> bool:
    return cpu_usage > THRESHOLD_CPU

def has_crossed_threshold_mem(memory_usage) -> bool:
    return memory_usage > THRESHOLD_MEMORY

def get_status():
    cpu_usage = psutil.cpu_percent(interval=1)
    memory_usage = psutil.virtual_memory().percent
    
    return cpu_usage, memory_usage

def store_usage(cpu_usage, memory_usage):
    with open(USAGE_LOG_PATH, mode='a', newline='') as file:
        writer = csv.writer(file)
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        writer.writerow([timestamp, cpu_usage, memory_usage])

if __name__ == "__main__":
    try:
        with open(CONFIG_PATH, 'r') as f:
            config = json.load(f)

        locals().update(config)
    except FileNotFoundError:
        print("No config file found, using default values.")
    except json.JSONDecodeError:
        print("Error reading config file, using default values.")
    os.makedirs(PATH, exist_ok=True)
    main()