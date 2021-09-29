Release Checklist
=================

 * Create code name at <https://killercup.github.io/codenamer/> "English Adjectives" "Animals" Alliterate
 * In ttauri-project/ttauri:
   - Create release branch
   - Add entry to CHANGELOG.md 
   - Update version in vcpkg.json
   - Merge into main (needed for ttauri-project/ttauri\_hello\_world test builds)
 * In ttauri-project/ttauri\_hello\_world:
   - Create release branch
   - Fix hello world application with "ttauri-dev x64-MSVC-Debug" mode.
   - Make pull request
   - Check if pull request builds on CI.
 * Make release of ttauri-project/ttauri
 * In ttauri-project/vcpkg
   - Update port file for new version
   - Create pull request to microsoft/vcpkg
   - Wait a few days for microsoft to merge
 * In ttauri-project/ttauri\_hello\_world
   - Merge pull request
   - Make release
   - Create installable

