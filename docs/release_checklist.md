Release Checklist
=================

 * Create code name at <https://killercup.github.io/codenamer/> "English Adjectives" "Animals" Alliterate
 * In hikogui/hikogui:
   - Create release branch
   - Add entry to CHANGELOG.md 
   - Update version in vcpkg.json
   - Merge into main (needed for hikogui/hikogui\_hello\_world test builds)
 * In hikogui/hikogui\_hello\_world:
   - Create release branch
   - Update version in vcpkg.json
   - Fix hello world application with "hikogui-dev x64-MSVC-Debug" mode.
   - Run and check both debug and release build
   - Make pull request
   - Check if pull request builds on CI.
 * Make release of hikogui/hikogui
   - Copy the release description from CHANGELOG.md
 * In hikogui/hikogui.github.io Run the "Documentation Pipeline"
 * In hikogui/vcpkg
   - Update ports/hikogui/vcpkg.json
   - Update ports/hikogui/portfile.cmake (update REF, set SHA512 to 0)
   - run `.\vcpkg.exe install hikogui --triplet x64-windows-static` to get the SHA512
   - Update ports/hikogui/portfile.cmake with the new SHA512
   - run `.\vcpkg.exe install hikogui --triplet x64-windows-static` to test if building works.
   - `.\vcpkg.exe format-manifest --all`
   - commit the changes before doing x-add-version
   - `.\vcpkg.exe x-add-version hikogui --overwrite-version`
   - commit the changes to the version files
   - run `.\vcpkg.exe remove hikogui --triplet x64-windows-static`
   - run `.\vcpkg.exe install hikogui --triplet x64-windows-static` should be cached install
   - Merge two commits `git rebase -i HEAD~2` (squash second commit) (Name commit `[hikogui] update to version x.x.x`)
   - Create pull request to microsoft/vcpkg
   - Wait a few days for microsoft to merge
 * In hikogui/hikogui\_hello\_world
   - Merge pull request
   - Make release
   - Download the ecpack artifact.
   - Create an install executable with ecpack-and-sign, and add the executable to the release.
 * Publish release
   - Make custom message for reddit/r/cpp, post as link + follow-up post
   - Make custom message for twitter include \#cplusplus

