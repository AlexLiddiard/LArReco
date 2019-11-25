#!/bin/bash
git config user.name "JaBCo1"
git config user.email "J.Collings@warwick.ac.uk"
git add -A
git reset -- "bin/*"
git reset -- "build/*"
git reset -- "lib/*"
git reset -- ".vscode/*"
git commit -m "$1"
git push --force origin master
