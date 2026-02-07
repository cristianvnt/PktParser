#!/usr/bin/env python3
import requests
import psycopg2
import sys
from pathlib import Path
from datetime import datetime
from dotenv import load_dotenv
import os

SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent

load_dotenv(PROJECT_ROOT / '.env')

DB_CONFIG = \
{
    'host': os.getenv('POSTGRES_HOST', 'localhost'),
    'port': os.getenv('POSTGRES_PORT', '5432'),
    'dbname': os.getenv('POSTGRES_DB', 'wow_metadata'),
    'user': os.getenv('POSTGRES_USER', 'wowparser'),
    'password': os.getenv('POSTGRES_PASSWORD')
}

GITHUB_API_BASE = "https://api.github.com/repos/mdX7/ngdp_data"

def fetch_build_list() -> list[int]:
    url = f"{GITHUB_API_BASE}/contents/EU/wow"
    
    try:
        response = requests.get(url, timeout=30)
        
        if response.status_code == 403:
            print("Error: GitHub API rate limit exceeded")
            print("Wait 1 hour or use GitHub token")
            return []
        
        if response.status_code != 200:
            print(f"Error: GitHub API returned {response.status_code}")
            return []
        
        contents = response.json()
        
        if not isinstance(contents, list):
            print("Error: Unexpected API response format")
            return []
        
        builds = []
        for item in contents:
            if item.get('type') == 'dir':
                name = item.get('name', '')
                if name.isdigit():
                    build_num = int(name)
                    if 1000 <= build_num <= 99999:  # valid build number range
                        builds.append(build_num)
        
        return sorted(builds, reverse=True)
    
    except requests.RequestException as e:
        print(f"Error: Network request failed - {e}")
        return []

def fetch_deploy_timestamp(build_number: int) -> datetime | None:
    # fetch deploy timestamp for a build from commit history
    url = f"{GITHUB_API_BASE}/commits"
    params = {
        'path': f'EU/wow/{build_number}',
        'per_page': 1
    }
    
    try:
        response = requests.get(url, params=params, timeout=10)
        
        if response.status_code != 200:
            return None
        
        commits = response.json()
        
        if not commits or len(commits) == 0:
            return None
        
        timestamp_str = commits[0]['commit']['committer']['date']
        return datetime.fromisoformat(timestamp_str.replace('Z', '+00:00'))
    
    except Exception:
        return None

def update_database(builds_with_timestamps: list[dict]) -> tuple[int, int]:
    conn = psycopg2.connect(**DB_CONFIG)
    cursor = conn.cursor()
    
    updated = 0
    skipped = 0
    skipped_builds = []
    
    for item in builds_with_timestamps:
        build_num = item['build']
        deploy_time = item['timestamp']
        
        cursor.execute("SELECT 1 FROM builds WHERE build_number = %s", (build_num,))
        
        if cursor.fetchone() is None:
            skipped += 1
            skipped_builds.append(build_num)
            continue
        
        cursor.execute("""
            UPDATE builds 
            SET deploy_timestamp = %s 
            WHERE build_number = %s AND deploy_timestamp IS NULL
        """, (deploy_time, build_num))
        
        if cursor.rowcount > 0:
            updated += 1
    
    conn.commit()
    conn.close()
    
    return updated, skipped, skipped_builds

def main():
    print("Fetching WoW build deploy timestamps")
    print(f"Source: {GITHUB_API_BASE}")
    print()
    
    start_idx = 0
    end_idx = 10  # default: fetch 10 most recent builds
    
    if len(sys.argv) >= 3:
        start_idx = int(sys.argv[1])
        end_idx = int(sys.argv[2])
    
    print("Fetching build list from GitHub...")
    all_builds = fetch_build_list()
    
    if not all_builds:
        print("Failed to fetch build list")
        sys.exit(1)
    
    print(f"Found {len(all_builds)} total builds")
    print()
    
    selected_builds = all_builds[start_idx:end_idx]
    
    if not selected_builds:
        print(f"No builds in range [{start_idx}:{end_idx}]")
        sys.exit(0)
    
    print(f"Fetching timestamps for {len(selected_builds)} builds (range [{start_idx}:{end_idx}])...")
    print()
    
    builds_with_timestamps = []
    
    for i, build_num in enumerate(selected_builds, 1):
        print(f"  [{i}/{len(selected_builds)}] Build {build_num}... ", end="", flush=True)
        
        deploy_time = fetch_deploy_timestamp(build_num)
        
        if deploy_time:
            builds_with_timestamps.append({
                'build': build_num,
                'timestamp': deploy_time
            })
            print(f"{deploy_time.strftime('%Y-%m-%d %H:%M:%S')}")
        else:
            print("failed")
    
    print()
    print(f"Successfully fetched {len(builds_with_timestamps)}/{len(selected_builds)} timestamps")
    
    if not builds_with_timestamps:
        print("No timestamps to update")
        sys.exit(0)
    
    print()
    print("Updating database...")
    
    try:
        updated, skipped, skipped_builds = update_database(builds_with_timestamps)
        print(f"  Updated: {updated}")
        print(f"  Skipped (not in DB): {skipped}")
        if skipped_builds:
            print(f"  Skipped builds: {', '.join(map(str, skipped_builds))}")
        print()
        print("Complete")
    except psycopg2.Error as e:
        print(f"Error: Database update failed")
        print(f"Details: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()