#!/usr/bin/env python3
"""
Generate objdiff.json configuration for progress tracking.
This script reads the splat YAML config and creates an objdiff.json
that maps source files to their target and base objects.
"""

import json
import yaml
from pathlib import Path

# Project paths
PROJECT_ROOT = Path(__file__).parent.parent.parent
CONFIG_PATH = PROJECT_ROOT / "config" / "SLUS_010.13.yaml"
OUTPUT_PATH = PROJECT_ROOT / "objdiff.json"

def load_splat_config():
    """Load the splat YAML configuration."""
    with open(CONFIG_PATH, 'r') as f:
        return yaml.safe_load(f)

def is_psyq_file(file_name: str) -> bool:
    """Check if the file belongs to the psyq library folder."""
    # Matches psyq/something.c, psyq/libc/something.c, etc.
    parts = file_name.split('/')
    return 'psyq' in parts


def generate_objdiff_config():
    """Generate objdiff.json from splat config."""
    config = load_splat_config()
    
    # Extract options
    options = config.get('options', {})
    game_name = options.get('basename', 'slus_010.13')
    
    # Units list - one for each C file in the code segments
    units = []
    
    # Parse segments to find code segments with C files
    segments = config.get('segments', [])
    
    for segment in segments:
        # Skip if segment is a list (e.g. [0x2F800])
        if not isinstance(segment, dict):
            continue
            
        if segment.get('type') == 'code':
            subsegments = segment.get('subsegments', [])
            
            for subseg in subsegments:
                # Check if it's a C file subsegment
                # Format: [offset, 'c', name] or similar
                if isinstance(subseg, list) and len(subseg) >= 3 and subseg[1] == 'c':
                    file_name = subseg[2]
                    
                    # Skip psyq / SDK files
                    if is_psyq_file(file_name):
                        continue
                    
                    # Create unit entry
                    unit = {
                        "name": f"main/{file_name}",
                        "target_path": f"build/asm/{file_name}.o",
                        "base_path": f"build/src/{file_name}.o"
                    }
                    units.append(unit)
    
    # Create objdiff.json structure
    objdiff_config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "make objdiff-objects",
        "build_target": True,
        "build_base": True,
        "units": units
    }
    
    # Write to objdiff.json
    with open(OUTPUT_PATH, 'w') as f:
        json.dump(objdiff_config, f, indent=2)
    
    print(f"Generated {OUTPUT_PATH}")
    print(f"Found {len(units)} units (psyq files excluded):")
    for unit in units:
        print(f"  - {unit['name']}")


if __name__ == "__main__":
    generate_objdiff_config()