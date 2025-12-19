#!/usr/bin/env python3
import re
import os
import requests
import psycopg2
import sys
from pathlib import Path
from datetime import datetime
from dotenv import load_dotenv

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

def FetchBuildDirectories():
    url = "https://api.github.com/repos/mdX7/ngdp_data/contents/EU/wow"
    
    try:
        response = requests.get(url, timeout=30)
        
        print(f"  Status: {response.status_code}")
        
        if response.status_code == 403:
            print("  Too many requests :( - wait 1 hour")
            return []
        
        if response.status_code != 200:
            print(f"  Error: {response.status_code}")
            return []
        
        contents = response.json()
        
        if not isinstance(contents, list):
            print(f"  Error: Unexpected response type")
            return []
        
        builds = []
        
        for item in contents:
            if item.get('type') == 'dir':
                name = item.get('name', '')
                if name.isdigit():
                    buildNum = int(name)
                    if 1000 <= buildNum <= 99999:
                        builds.append(buildNum)
        
        buildList = sorted(builds, reverse=True)
        print(f"  Found {len(buildList)} builds")
        
        return buildList
    
    except Exception as e:
        print(f"  Exception: {e}")
        return []

def FetchBuildDeployDate(buildNumber):
    url = f"https://api.github.com/repos/mdX7/ngdp_data/commits"
    params = {
        'path': f'EU/wow/{buildNumber}',
        'per_page': 1
    }
    
    try:
        response = requests.get(url, params=params, timeout=10)
        
        if response.status_code != 200:
            return None
        
        commits = response.json()
        
        if not commits or len(commits) == 0:
            return None
        
        timestamp = commits[0]['commit']['committer']['date']
        return datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
    
    except Exception as e:
        return None

def UpdateDeployTimestamps(builds):
    conn = psycopg2.connect(**DB_CONFIG)
    cursor = conn.cursor()
    
    updated = 0
    skipped = 0
    
    for build in builds:
        cursor.execute("SELECT 1 FROM builds WHERE build_number = %s", (build['build'],))
        
        if cursor.fetchone() is None:
            print(f"Build {build['build']} skipped")
            skipped += 1
            continue
        
        cursor.execute(
        """
            UPDATE builds 
            SET deploy_timestamp = %s 
            WHERE build_number = %s AND deploy_timestamp IS NULL
        """, (build['deployed'], build['build']))
        
        if cursor.rowcount > 0:
            updated += 1
    
    conn.commit()
    conn.close()
    
    return updated, skipped

def Main():
    print("Source: mdX7/ngdp_data/EU/wow")
    print("")
    
    startIdx = 0
    endIdx = 10
    
    if len(sys.argv) >= 3:
        startIdx = int(sys.argv[1])
        endIdx = int(sys.argv[2])
    
    print("Fetching build list...")
    buildNumbers = FetchBuildDirectories()
    
    if not buildNumbers:
        print("  No builds found or rate limited")
        sys.exit(1)
    
    print(f"  Total: {len(buildNumbers)} builds")
    print("")
    
    recentBuilds = buildNumbers[startIdx:endIdx]
    
    if not recentBuilds:
        print(f"  No builds in range [{startIdx}:{endIdx}]")
        sys.exit(0)
    
    print(f"Fetching timestamps for builds {startIdx}-{endIdx} ({len(recentBuilds)} builds)...")
    print("")
    
    buildsWithDates = []
    
    for i, buildNum in enumerate(recentBuilds, 1):
        print(f"  [{i}/{len(recentBuilds)}] Build {buildNum}...", end=" ", flush=True)
        
        deployDate = FetchBuildDeployDate(buildNum)
        
        if deployDate:
            buildsWithDates.append({'build': buildNum, 'deployed': deployDate})
            print(f"{deployDate.strftime('%Y-%m-%d')}")
        else:
            print("Failed")
    
    print(f"Successfully fetched {len(buildsWithDates)} timestamps")
    
    if not buildsWithDates:
        sys.exit(0)
    
    print("Updating database...")
    updated, skipped = UpdateDeployTimestamps(buildsWithDates)
    
    print(f"  Updated: {updated}")
    print(f"  Skipped (not in DB): {skipped}")
    print("")
    print("Complete")

if __name__ == "__main__":
    Main()