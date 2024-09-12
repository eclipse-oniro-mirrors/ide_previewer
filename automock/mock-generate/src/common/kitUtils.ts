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
import ts from 'typescript';
import { ImportElementEntity } from '../declaration-node/importAndExportDeclaration';

export interface KitImportInfo {
  fromPath: string;
  isExport: string[];
  isDefaultExport: string;
}

// kit file map
export const KIT_MAP = new Map<string, KitImportInfo[]>();

/**
 * get all kit files
 * @param kitDir root path
 */
export function collectAllKitFiles(kitDir: string): void {
  const arr = fs.readdirSync(kitDir);
  arr.forEach(value => {
    const fullPath = path.join(kitDir, value);
    const stats = fs.statSync(fullPath);
    if (stats.isDirectory()) {
      collectAllKitFiles(fullPath);
    } else {
      let fileBaseName: string;
      if (value.endsWith('.d.ts')) {
        fileBaseName = path.basename(value, '.d.ts');
      }
      if (fileBaseName) {
        handleKitFile(fullPath, fileBaseName);
      }
    }
  });
}

/**
 * kit file convert to ast
 * @param kitPath kit file path
 * @param fileName kit file name
 */
function handleKitFile(kitPath: string, fileName: string): void {
  KIT_MAP.set(fileName, []);

  const code = fs.readFileSync(kitPath);
  const sourceFile = ts.createSourceFile(kitPath, code.toString(), ts.ScriptTarget.Latest);
  sourceFile.forEachChild(node => {
    if (ts.isImportDeclaration(node)) {
      KIT_MAP.get(fileName).push(getKitImportDeclaration(node));
    }
  });
}

/**
 * get kit file import declaration
 * @param importNode kit file import node
 * @returns KitImportInfo
 */
export function getKitImportDeclaration(importNode: ts.ImportDeclaration): KitImportInfo {
  let isDefaultExport = '';
  const isExport = [];
  const moduleSpecifier = importNode.moduleSpecifier as ts.StringLiteral;
  const fromPath = moduleSpecifier.text.trim();
  const importClause = importNode.importClause;
  if (importClause !== undefined) {
    if (importClause.name) {
      isDefaultExport = importClause.name.text.trim();
    }
    importClause.namedBindings?.forEachChild((node: ts.ImportSpecifier) => {
      if (node.name) {
        isExport.push(node.name.text.trim());
      }
    });
  }

  return {
    fromPath,
    isDefaultExport,
    isExport
  };
}

/**
 * handle kit file import map info
 * @param importEntity import entity data
 * @returns ImportElementEntity[]
 */
export function handleImportKit(importEntity: ImportElementEntity): ImportElementEntity[] {
  const importPath = importEntity.importPath.replace(/(\'|\")/g, '');
  if (!importPath.startsWith('@kit.')) {
    return [];
  }
  const importEntities: ImportElementEntity[] = [];
  let importElements: string[];
  if (importEntity.importElements.startsWith('{') && importEntity.importElements.endsWith('}')) {
    importElements = importEntity.importElements
      .substring(1, importEntity.importElements.length - 1)
      .split(',')
      .map(element => element.trim());
  }

  const kitImportInfos: KitImportInfo[] = KIT_MAP.get(importPath);
  if (!kitImportInfos) {
    throw new Error(`Can't find kitFileInfos from ${importPath}`);
  }

  importElements.forEach(element => {
    importEntities.push(getKitImportInfo(kitImportInfos, element, importPath));
  });
  return importEntities;
}

/**
 * get Kit file import map info
 * @param kitImportInfos map infos
 * @param element import entity info
 * @param importPath import file path
 * @returns ImportElementEntity
 */
function getKitImportInfo(kitImportInfos: KitImportInfo[], element: string, importPath: string): ImportElementEntity {
  let defaultImport: string;
  let commonImport: string;
  const kitImportInfo = kitImportInfos.find(kitInfo => {
    if (element === kitInfo.isDefaultExport) {
      defaultImport = kitInfo.isDefaultExport;
      return true;
    } else if (kitInfo.isExport.includes(element)) {
      commonImport = element;
      return true;
    } else {
      return false;
    }
  });

  if (!kitImportInfo) {
    throw new Error(`Can't find ${element} from ${importPath}`);
  }

  const defaultImportStr = defaultImport ? `${defaultImport}${commonImport ? ',' : ''}` : '';
  const commonImportsStr = commonImport ? `{ ${commonImport} }` : '';
  return {
    importElements: `${defaultImportStr} ${commonImportsStr}`,
    importPath: kitImportInfo.fromPath
  };
}
