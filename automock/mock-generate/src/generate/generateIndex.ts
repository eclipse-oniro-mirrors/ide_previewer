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

import path from 'path';
import { isDeclarationFile, isNeedMocked } from '../common/commonUtils';
import {
  arktsDtsFileList,
  mockBufferMap,
  NO_CONTENT_FILES,
  ohosDtsFileList
} from '../common/constants';

/**
 * 声测会给你index文件
 * @param indexFilePath index文件输出路径
 * @returns
 */
export function generateIndex(indexFilePath: string): string {
  let indexBody = '';
  let caseBody = '';

  [].forEach(fileName => {
    if (NO_CONTENT_FILES.has(fileName)) {
      return;
    }
    if (!isDeclarationFile(fileName)) {
      return;
    }
    if (!isNeedMocked(fileName)) {
      return;
    }

    const mockBuffer = mockBufferMap.get(fileName);
    const mockedFilePath = mockBuffer.mockedFilePath;
    const fileBaseName = path.basename(mockedFilePath, '.js');
    const relativePath = path.relative(path.dirname(indexFilePath), mockedFilePath).replace(/\.js$/, '').replace(/\\/g, '/');
    const asName = path.basename(fileBaseName).replace(/^@/, '').replace(/\./g, '_');
    if (mockBuffer?.contents.members.default && mockBuffer?.contents.members.default.isNeedMock) {
      indexBody += `import ${asName} from './${relativePath}';\n`;
    } else {
      indexBody += `import * as ${asName} from './${relativePath}';\n`;
    }

    caseBody += `case '${fileBaseName.replace(/^@(ohos\.)?/, '')}':\n\treturn ${asName};\n`;
  });

  indexBody += `export function mockRequireNapiFun() {
    global.requireNapi = function(...args) {
      const globalNapi = global.requireNapiPreview(...args);
      if (globalNapi !== undefined) {
        return globalNapi;
      }
      switch (args[0]) {`;
  indexBody += caseBody;
  const endBody = `}
      if (global.hosMockFunc !== undefined) {
        return global.hosMockFunc(args[0]);
      }
          }
        }`;
  indexBody += endBody;
  return indexBody;
}
