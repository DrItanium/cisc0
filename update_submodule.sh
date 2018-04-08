#!/bin/bash
pushd misc/forth_interpreter
git pull origin master
popd
git commit -m "Submodule update" misc/forth_interpreter
git push origin master
