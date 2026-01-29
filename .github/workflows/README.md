# GitHub Actions Workflows

This directory contains GitHub Actions workflows for automated building and progress tracking.

## Workflows

### 1. `progress.yml` - Main Progress Report Workflow
**Triggers:** Push to main/master, Pull Requests, Manual dispatch

This is the primary workflow that:
1. Builds the Docker image
2. Runs splat to generate assembly
3. Builds the project
4. Builds objdiff objects (target + base)
5. Generates objdiff.json
6. Downloads objdiff-cli
7. Generates progress report
8. Uploads artifacts

**Use this for:** Main branch builds and generating progress reports for decomp.dev

### 2. `progress-cached.yml` - Optimized with Caching
**Triggers:** Push to main/master, Pull Requests, Manual dispatch

Same as `progress.yml` but with Docker layer caching and pip caching for faster builds.

**Differences from progress.yml:**
- Caches Docker layers between runs (faster rebuilds)
- Caches Python dependencies
- Runs splat outside Docker (faster)
- Better suited for frequent builds

**Use this for:** Projects with frequent commits where build speed matters

### 3. `pr-check.yml` - Quick PR Validation
**Triggers:** Pull Requests only

Lightweight workflow that only verifies the build works. Skips objdiff and reporting.

**Use this for:** Fast feedback on PRs without the overhead of full reporting

## Artifacts Generated

All workflows that complete successfully will upload artifacts:

### Progress Report Artifact
- **Name:** `SLUS_010.13_report`
- **Contents:** `build/report.json`
- **Purpose:** Progress tracking for decomp.dev

### Build Artifact (optional)
- **Name:** `SLUS_010.13_build`
- **Contents:** `build/slus_010.13.elf`, `build/slus_010.13.map`
- **Purpose:** Downloadable build artifacts

## Which Workflow Should I Use?

### For New Projects (Recommended)
Start with **`progress.yml`** - it's simple and reliable.

### For Active Projects
Use **`progress-cached.yml`** - it's faster due to caching.

### For Both
Enable **`pr-check.yml`** in addition to either of the above for fast PR feedback.

## Workflow Comparison

| Feature | progress.yml | progress-cached.yml | pr-check.yml |
|---------|-------------|---------------------|--------------|
| Docker caching | ❌ | ✅ | ❌ |
| Pip caching | ❌ | ✅ | ✅ |
| Splat in Docker | ✅ | ❌ (runs on host) | ❌ (runs on host) |
| Full report | ✅ | ✅ | ❌ |
| Speed | Normal | Fast | Fastest |
| Complexity | Simple | Medium | Simple |

## Manual Triggering

All workflows can be manually triggered from the GitHub Actions tab using the "Run workflow" button.

## decomp.dev Integration

Once your workflows are running and uploading `SLUS_010.13_report` artifacts:

1. Go to https://decomp.dev/manage/new
2. Add your repository
3. decomp.dev will automatically track your progress!

The report artifact name **must** match the pattern `{VERSION}_report` where `{VERSION}` is your game version (e.g., `SLUS_010.13`).

## Testing Locally

Before pushing, you can test the workflow steps locally:

```bash
# 1. Build Docker image
docker build -t lom-decomp .

# 2. Run splat
docker run --rm -v $(pwd):/lom lom-decomp \
  python3 -m splat split config/SLUS_010.13.yaml --make-full-disasm-for-code

# 3. Build project
docker run --rm -v $(pwd):/lom lom-decomp make all

# 4. Build objdiff objects
docker run --rm -v $(pwd):/lom lom-decomp make objdiff-objects

# 5. Generate objdiff.json
python tools/generate_objdiff_config.py

# 6. Generate report
./tools/objdiff/objdiff-cli-linux-x86_64 report generate -o build/report.json
```

## Troubleshooting

### Build fails with "no such file or directory"
- Make sure all submodules are initialized: `git submodule update --init --recursive`

### objdiff-cli not found
- The workflow downloads it automatically
- For local testing, download from: https://github.com/encounter/objdiff/releases

### Report not generated
- Check that `objdiff.json` exists in the repository root
- Verify that both target and base objects were built
- Look at the workflow logs for specific errors

### Artifact upload fails
- Ensure `build/report.json` exists before upload step
- Check file permissions in Docker container

## Customization

### Change Game Version
If you have multiple game versions, modify the artifact name in the workflow:

```yaml
- name: Upload progress report
  uses: actions/upload-artifact@v4
  with:
    name: YOUR_VERSION_report  # Change this
    path: build/report.json
```

### Add More Build Steps
Insert additional steps before or after existing ones:

```yaml
- name: Custom step
  run: |
    # Your custom commands here
```

### Change Build Triggers
Modify the `on:` section to change when workflows run:

```yaml
on:
  push:
    branches: [ main, dev ]  # Add more branches
  schedule:
    - cron: '0 0 * * *'  # Daily at midnight
```
