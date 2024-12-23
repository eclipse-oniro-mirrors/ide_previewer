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

import fs from 'fs';
import path from 'path';
import { createSourceFile, ScriptTarget } from 'typescript';
import {
  getOhosInterfacesDir,
  isDeclarationFile,
  mkdirsSync,
  generateKeyValue,
  isNeedMocked
} from './common/commonUtils';
import { getSourceFileAssembly } from './declaration-node/sourceFileElementsAssemply';
import { generateEntry } from './generate/generateEntry';
import { generateIndex } from './generate/generateIndex';
import { generateSystemIndex } from './generate/generateSystemIndex';
import {
  arktsDtsFileList,
  componentDtsFileList,
  D_ETS, D_TS,
  KeyValueTypes,
  kitsDtsFileList,
  mockBufferMap,
  MockedFileMap,
  NO_CONTENT_FILES,
  ohosDtsFileList,
  RawFileMap
} from './common/constants';
import { ApiFolder, MockBuffer } from './types';
import { generateContent, handleDeclares } from './generate/generateContent';

/**
 * 获取所有接口文件路径
 * @param dir 接口目录路径
 * @param fileList 接口文件列表
 * @param isHmsDtsFile 是否是harmonyOS接口
 * @returns
 */
function getAllDtsFile(dir: string, fileList: string[], isHmsDtsFile: boolean): string[] {
  const arr = fs.readdirSync(dir);
  if (!dir.toString().includes('node_modules') && !dir.toString().includes(path.join('@internal', 'component'))) {
    arr.forEach(value => {
      const fullPath = path.join(dir, value);
      const stats = fs.statSync(fullPath);
      if (stats.isDirectory()) {
        getAllDtsFile(fullPath, fileList, isHmsDtsFile);
      } else {
        if (!isDeclarationFile(value)) {
          return;
        }
        fileList.push(fullPath);
      }
    });
  }
  return fileList;
}

/**
 * 获取所有组件接口文件路径
 * @param dir 组件目录路径
 * @returns
 */
function getAllComponentsFilePath(dir: string): Array<string> {
  const componentPath = path.join(dir, '@internal', 'component', 'ets');
  if (!fs.existsSync(componentPath)) {
    return;
  }
  const componentPathArr = fs.readdirSync(componentPath);
  componentPathArr.forEach(value => {
    const fullPath = path.join(componentPath, value);
    if (fs.existsSync(fullPath) && !fs.statSync(fullPath).isDirectory()) {
      if (!isDeclarationFile(value)) {
        return;
      }
      componentDtsFileList.push(fullPath);
    }
  });
}

/**
 * 获取所有global接口文件路径
 * @param dir global目录路径
 */
function getAllGlobalFilePath(dir: string): Array<string> {
  const globalPath = path.join(dir, 'global');
  if (!fs.existsSync(globalPath)) {
    return;
  }
  const globalPathArr = fs.readdirSync(globalPath);
  globalPathArr.forEach(value => {
    const fullPath = path.join(globalPath, value);
    if (fs.existsSync(fullPath) && !fs.statSync(fullPath).isDirectory()) {
      if (!isDeclarationFile(value)) {
        return;
      }
      ohosDtsFileList.push(fullPath);
    }
  });
}

/**
 * 获取mock后的接口文件路径
 * @param beforeMockedFilePath mock钱的文件路径
 * @param apiFolder api目录路径
 */
function getMockedFileName(beforeMockedFilePath: string, apiFolder: ApiFolder): string {
  const outMockJsFolder = path.join(__dirname, '..', '..', 'runtime', 'main', 'extend', 'systemplugin');
  let fileName: string;
  if (beforeMockedFilePath.endsWith(D_TS)) {
    fileName = path.basename(beforeMockedFilePath, D_TS);
  } else if (beforeMockedFilePath.endsWith(D_ETS)) {
    fileName = path.basename(beforeMockedFilePath, D_ETS);
  } else {
    return '';
  }

  let tmpOutputMockJsFileDir = outMockJsFolder;
  if (!fileName.startsWith('@system.')) {
    tmpOutputMockJsFileDir = path.join(outMockJsFolder, 'napi', apiFolder);
  }
  let dirName: string;
  switch (apiFolder) {
    case 'api': {
      dirName = path.join(tmpOutputMockJsFileDir, path.dirname(beforeMockedFilePath).split(`${path.sep}api`)[1]);
      break;
    }
    case 'component': {
      dirName = tmpOutputMockJsFileDir;
      break;
    }
    case 'arkts': {
      dirName = path.join(tmpOutputMockJsFileDir, path.dirname(beforeMockedFilePath).split(`${path.sep}arkts`)[1]);
      break;
    }
    case 'kits': {
      dirName = path.join(tmpOutputMockJsFileDir, path.dirname(beforeMockedFilePath).split(`${path.sep}kits`)[1]);
      break;
    }
  }

  if (!fs.existsSync(dirName)) {
    mkdirsSync(dirName);
  }
  return path.join(dirName, fileName + '.js');
}

/**
 * 初始化接口文件mock信息
 * @param typeFile 类型文件路径
 * @param apiFolder api目录路径
 */
function initialTypeFile(typeFile: string, apiFolder: ApiFolder): void {
  if (!isDeclarationFile(typeFile)) {
    return;
  }
  const filePath = getMockedFileName(typeFile, apiFolder);
  if (mockBufferMap.get(typeFile)) {
    return;
  }
  const mockBuffer: MockBuffer = {
    rawFilePath: typeFile,
    mockedFilePath: filePath,
    contents: generateKeyValue(filePath, KeyValueTypes.FILE)
  };
  mockBufferMap.set(typeFile, mockBuffer);
  RawFileMap.set(typeFile, filePath);
  MockedFileMap.set(filePath, typeFile);
}

/**
 * 处理所有ets文件的mock逻辑
 */
function etsFileToMock(): void {
  componentDtsFileList.forEach(file => initialTypeFile(file, 'component'));
  ohosDtsFileList.forEach(file => initialTypeFile(file, 'api'));
  arktsDtsFileList.forEach(file => initialTypeFile(file, 'arkts'));
  kitsDtsFileList.forEach(file => initialTypeFile(file, 'kits'));

  const allFileList = [
    ...componentDtsFileList,
    ...ohosDtsFileList,
    ...arktsDtsFileList,
    ...kitsDtsFileList
  ];

  allFileList.forEach(file => {
    const code = fs.readFileSync(file);
    const sourceFile = createSourceFile(file, code.toString().replace(/ struct /g, ' class '), ScriptTarget.Latest);
    const mockBuffer = mockBufferMap.get(file);
    getSourceFileAssembly(sourceFile, mockBuffer, mockBuffer.contents.members, mockBuffer.contents);
  });

  handleDeclares(path.join(__dirname, '..', '..', 'runtime', 'main', 'extend', 'systemplugin'));

  ohosDtsFileList.forEach(filepath => {
    if (!isDeclarationFile(filepath)) {
      return;
    }
    if (!isNeedMocked(filepath)) {
      return;
    }
    const mockBuffer = mockBufferMap.get(filepath);
    const keyValue = mockBuffer.contents;
    const mockedContents = generateContent(mockBuffer, keyValue.members);
    if (!mockedContents.length) {
      NO_CONTENT_FILES.add(filepath);
    } else {
      fs.writeFileSync(mockBuffer.mockedFilePath, mockedContents);
    }
  });
}

/**
 * 项目入口函数
 */
function main(): void {
  const outMockJsFileDir = path.join(__dirname, '..', '..', 'runtime', 'main', 'extend', 'systemplugin');
  getAllGlobalFilePath(getOhosInterfacesDir());
  getAllComponentsFilePath(getOhosInterfacesDir());
  getAllDtsFile(getOhosInterfacesDir(), ohosDtsFileList, false);
  getAllDtsFile(path.resolve(getOhosInterfacesDir(), '..', 'arkts'), arktsDtsFileList, false);
  getAllDtsFile(path.resolve(getOhosInterfacesDir(), '..', 'kits'), kitsDtsFileList, false);

  etsFileToMock();

  if (!fs.existsSync(path.join(outMockJsFileDir, 'napi'))) {
    mkdirsSync(path.join(outMockJsFileDir, 'napi'));
  }
  const napiIndexPath = path.join(outMockJsFileDir, 'napi', 'index.js');
  fs.writeFileSync(napiIndexPath, generateIndex(napiIndexPath));
  fs.writeFileSync(path.join(outMockJsFileDir, 'index.js'), generateSystemIndex());
  fs.writeFileSync(path.join(outMockJsFileDir, 'entry.js'), generateEntry());
}

main();
