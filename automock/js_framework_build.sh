#!/bin/bash
# Copyright (c) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e
echo "copy source code..."
prebuilts_path=${9}
# copy dependency file to generate dir of gn
# the params come from .gn

# copy runtime to target out, and runtime/css-what is solt link, copy it always follow symbolic links in SOURCE
if [ "${8}" == 'true' ];then
  #cp -R -L $3 $8
  if [ "${12}" == 'true' ];then
    cp -R ${11} $7
  fi
else
  #cp -r -L $3 $8
  if [ "${12}" == 'true' ];then
    cp -r ${11} $7
  fi
fi

# $2 => node $3 => node_modules
cp -f $4 $7

if [ -d "$prebuilts_path" ]; then
  echo "copy node_modules..."
  if [ "${9}" == 'true' ];then
    cp -R $3 $7
  else
    cp -r $3 $7
  fi
else
  echo "download node_modules..."
  npm install
  cp -r ./node_modules ../../tools_previewer/automock/mock-generate
fi

cp -f $5 $7
cp -f $6 $7
cp -f $1 $7
cp -f ${13} $7
if [ -d "$prebuilts_path" ]; then
  echo "prebuilts exists"
  # address problme of parallzing compile
  rm -rf "$7/current"
  link_path=$(realpath $2)
  ln -s $link_path "$7/current"
  cd $7
  if [ "${8}" == 'true' ];then
    if [ "${12}" == 'true' ];then
      ./current/bin/node ./mock-generate/build.js ${10}
    fi
    ./current/bin/node build_jsmock_system_plugin.js || exit 1 &
    wait
  else
    if [ "${12}" == 'true' ];then
      ./current/bin/node ./mock-generate/build.js ${10}
    fi
    ./current/bin/node build_jsmock_system_plugin.js || exit 1 &
    wait
  fi
else
  npm run build
fi

# after running, remove dependency file
rm -rf ./node_modules
if [ "${8}" == 'true' ];then
  rm -rf ./current
else
  rm -rf ./current
fi
rm -rf ./runtime
rm -rf ./tsconfig.json
rm -rf build_jsmock_system_plugin.js
rm -rf ./.eslintrc
rm -rf ./.babelrc
rm -rf ./package.json
if [ "${12}" == 'true' ];then
  rm -rf ./mock-generate
fi
