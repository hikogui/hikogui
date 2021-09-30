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
   - Update version in vcpkg.json
   - Fix hello world application with "ttauri-dev x64-MSVC-Debug" mode.
   - Make pull request
   - Check if pull request builds on CI.
 * Make release of ttauri-project/ttauri
   - Copy the release description from CHANGELOG.md
 * In ttauri-project/ttauri-project.github.io Run the "Documentation Pipeline"
 * In ttauri-project/vcpkg
   - Update ports/ttauri/vcpkg.json
   - Update ports/ttauri/portfile.cmake (update REF, set SHA512 to 0)
   - run `.\vcpkg.exe install ttauri --triplet x64-windows-static` to get the SHA512
   - Update ports/ttauri/portfile.cmake with the new SHA512
   - run `.\vcpkg.exe install ttauri --triplet x64-windows-static` to test if building works.
   - `.\vcpkg.exe format-manifest --all`
   - commit the changes before doing x-add-version
   - `.\vcpkg.exe x-add-version ttauri --overwrite-version`
   - commit the changes to the version files
   - run `.\vcpkg.exe remove ttauri --triplet x64-windows-static`
   - run `.\vcpkg.exe install ttauri --triplet x64-windows-static` should be cached install
   - Merge two commits `git rebase -i HEAD~2` (squash second commit) (Name commit `[ttauri] update to version x.x.x`)
   - Create pull request to microsoft/vcpkg
   - Wait a few days for microsoft to merge
 * In ttauri-project/ttauri\_hello\_world
   - Merge pull request
   - Make release
   - Create install executable.
 * Publish release
   - Make custom message for reddit/r/cpp, post as link + follow-up post
   - Make custom message for twitter include #cplusplus

