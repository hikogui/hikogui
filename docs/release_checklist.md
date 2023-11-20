Release Checklist
=================

 * Create code name at <https://killercup.github.io/codenamer/> "English Adjectives" "Animals" Alliterate
 * In hikogui/hikogui:
   - Create release branch
   - Add entry to CHANGELOG.md and update link section at end of file
   - Update version in vcpkg.json
   - (Possibly test and fix hikogui-hello-world in local mode).
   - Use the check_install tool:
     `PS C:\Users\Tjienta\Projects\hikogui\out\build\msvc-x64-windows-dbg> ..\..\..\tools\install_check\check_install.bat`
   - Merge into main (needed for hikogui/hikogui\_hello\_world test builds)
 * In hikogui/hikogui\_hello\_world:
   - Create release branch
   - Update version in vcpkg.json
   - Fix hello world application with "MSVC-x64-Debug (local)" mode.
   - Run and check both debug and release build
   - Make pull request
   - Check if the following builds succeed:
     - Build on Windows / x64-windows Debug (pull_request)
     - Build on Windows / x64-windows Release (pull_request)
     - Build on Windows (vcpkg) / x64-windows Debug (hikogui/vcpkg:head)
     - Build on Windows (vcpkg) / x64-windows Release (hikogui/vcpkg:head)
     - Build on Windows (vcpkg) / x64-windows-static Debug (hikogui/vcpkg:head)
     - Build on Windows (vcpkg) / x64-windows-static Release (hikogui/vcpkg:head)
 * Make release of hikogui/hikogui
   - Copy the release description from CHANGELOG.md
 * In hikogui/hikogui.github.io:
   - Run the "Documentation Pipeline" workflow
 * In hikogui/vcpkg ref:hikogui
   - fetch and merge vcpkg/vcpkg ref:master
   - Update ports/hikogui/vcpkg.json
   - Update ports/hikogui/portfile.cmake (update REF, set SHA512 to 0)
   - run `.\vcpkg.exe install hikogui --triplet x64-windows` to get the SHA512
   - Update ports/hikogui/portfile.cmake with the new SHA512
   - run `.\vcpkg.exe install hikogui --triplet x64-windows` to test if building works.
   - Commit to the branch `hikogui`
   - Rerun the actions from the hikogui\_hello\_world release branch and check if
     the following builds now succeed:
     - Build on Windows / x64-windows Debug (pull_request)
     - Build on Windows / x64-windows Release (pull_request)
     - Build on Windows (vcpkg) / x64-windows Debug (hikogui/vcpkg:head)
     - Build on Windows (vcpkg) / x64-windows Release (hikogui/vcpkg:head)
     - Build on Windows (vcpkg) / x64-windows-static Debug (hikogui/vcpkg:head)
     - Build on Windows (vcpkg) / x64-windows-static Release (hikogui/vcpkg:head)
     - Build on Windows (vcpkg) / x64-windows Debug (hikogui/vcpkg)
     - Build on Windows (vcpkg) / x64-windows Release (hikogui/vcpkg)
     - Build on Windows (vcpkg) / x64-windows-static Debug (hikogui/vcpkg)
     - Build on Windows (vcpkg) / x64-windows-static Release (hikogui/vcpkg)
   - `.\vcpkg.exe format-manifest --all`
   - commit the changes before doing x-add-version
   - `.\vcpkg.exe x-add-version hikogui --overwrite-version`
   - commit the changes to the version files
   - run `.\vcpkg.exe remove hikogui --triplet x64-windows`
   - run `.\vcpkg.exe install hikogui --triplet x64-windows` should be cached install
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

