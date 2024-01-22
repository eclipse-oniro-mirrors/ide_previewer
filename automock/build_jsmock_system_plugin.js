/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const fs = require('fs');

const path = require('path');

const rollup = require('rollup');

const resolve = require('rollup-plugin-node-resolve');

const commonjs = require('rollup-plugin-commonjs');

const json = require('rollup-plugin-json');

const babel = require('rollup-plugin-babel');

const typescript = require('rollup-plugin-typescript2');

const { uglify } = require('rollup-plugin-uglify');

const {
  eslint
} = require('rollup-plugin-eslint');

const frameworkBanner = 'var global=this; var process={env:{}}; ' + 'var setTimeout=global.setTimeout;\n';

const frameworkBannerForJSAPIMock = 'var global=globalThis;';

const onwarn = warning => {
  // Silence circular dependency warning
  if (warning.code === 'CIRCULAR_DEPENDENCY') {
    return;
  }
  console.warn(`(!) ${warning.message}`);
};

const tsPlugin = typescript({
  tsconfig: path.resolve(__dirname, 'tsconfig.json'),
  check: true
});

const esPlugin = eslint({
  include: ['**/*.ts'],
  exclude: ['node_modules/**', 'lib/**']
});

const configJSAPIMockInput = {
  input: path.resolve(__dirname, 'runtime/main/extend/systemplugin/index.js'),
  onwarn,
  plugins: [
    esPlugin,
    tsPlugin,
    json(),
    resolve(),
    commonjs(),
    babel({
      exclude: 'node_moduels/**'
    })
  ]
};

const configJSAPIMockOutput = {
  file: path.resolve(__dirname, 'dist/jsMockSystemPlugin.js'),
  format: 'umd',
  treeshake: false,
  banner: frameworkBannerForJSAPIMock
};

rollup.rollup(configJSAPIMockInput).then(bundle => {
  bundle.write(configJSAPIMockOutput).then(() => {
    countSize(configJSAPIMockOutput.file);
    const fileContent = fs.readFileSync(configJSAPIMockOutput.file, 'utf-8');
    fs.writeFileSync(configJSAPIMockOutput.file, fileContent.replace(/CommonMethod\$\d*/g, 'CommonMethod'), 'utf-8');
  });
});

function countSize(filePath) {
  const file = path.relative(__dirname, filePath);
  fs.stat(filePath, function (error, stats) {
    if (error) {
      console.error('file size is wrong');
    } else {
      const KB_BYTE_LENGTH = 1024;
      const num = 2;
      const size = (stats.size / KB_BYTE_LENGTH).toFixed(num) + 'KB';
      console.log(`generate snapshot file: ${file}...\nthe snapshot file size: ${size}...`);
    }
  });
}

