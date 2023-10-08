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

import type { TypeAliasEntity, TypeAliasTypeEntity } from '../declaration-node/typeAliasDeclaration';
import { firstCharacterToUppercase, getOhosInterfacesDir } from '../common/commonUtils';
import path from 'path';
import fs from 'fs';
import type { SourceFile } from 'typescript';

const interceptIndex = 2;

/**
 * generate type alias
 * @param typeAliasEntity
 * @param isInner
 * @param sourceFile
 * @param extraImport
 * @returns
 */
export function generateTypeAliasDeclaration(
  typeAliasEntity: TypeAliasEntity, isInner: boolean, sourceFile: SourceFile, extraImport: string[]
): string {
  let typeAliasName = '';
  if (!isInner) {
    typeAliasName += `export const ${typeAliasEntity.typeAliasName} = `;
  } else {
    typeAliasName += `const ${typeAliasEntity.typeAliasName} = `;
  }

  let typeAliasValue = '';

  const typeAliasTypeElements = typeAliasEntity.typeAliasTypeElements;

  if (typeAliasTypeElements) {
    typeAliasValue += parseImportExpression(typeAliasTypeElements, sourceFile, extraImport);
  }

  if (!typeAliasValue) {
    typeAliasValue += `'[PC Preview] unknown ${typeAliasEntity.typeAliasName}'`;
  }
  return typeAliasName + typeAliasValue + ';';
}

function getImportFileFullPath(typeName: string): string {
  const importRelatePathTmp = typeName.match(/\('[^'()]+'\)/);
  if (!importRelatePathTmp) {
    return '';
  }
  const importRelatePath = importRelatePathTmp[0].substring(interceptIndex, importRelatePathTmp[0].length - interceptIndex);
  const tmpRealPath = getOhosInterfacesDir() + importRelatePath.replace('../api', '').replace(/\//g, path.sep);
  if (fs.existsSync(tmpRealPath + '.d.ts')) {
    return tmpRealPath + '.d.ts';
  }

  if (fs.existsSync(tmpRealPath + '.d.ets')) {
    return tmpRealPath + '.d.ets';
  }
  console.warn(`Can not find import \'${importRelatePath}\'`);
  return '';
}

function pathToImportPath(currentFilePath: string, importFilePath: string): string {
  const currentFilePathSteps = currentFilePath.replace(/.d.e?ts/, '').split('/');
  const importFilePathSteps = importFilePath.replace(/.d.e?ts/, '').split(path.sep);
  const importFilePathStepsLength = importFilePathSteps.length;
  importFilePathSteps[importFilePathStepsLength - 1] = importFilePathSteps[importFilePathStepsLength - 1]
    .replace('@', '').replace(/\./g, '_');
  let differStepIndex: number;
  for (differStepIndex = 0; differStepIndex < currentFilePathSteps.length; differStepIndex++) {
    if (currentFilePathSteps[differStepIndex] !== importFilePathSteps[differStepIndex]) {
      break;
    }
  }
  const currentFileDifferPathSteps = currentFilePathSteps.slice(differStepIndex);
  const importFileDifferPathSteps = importFilePathSteps.slice(differStepIndex);
  if (currentFileDifferPathSteps.length === importFileDifferPathSteps.length
    && currentFileDifferPathSteps.length === 1) {
    return `./${path.basename(importFilePath)}`;
  } else {
    const steps = [];
    for (let i = 0; i < currentFileDifferPathSteps.length - 1; i++) {
      steps.push('..');
    }
    const fullSteps = steps.concat(importFileDifferPathSteps);
    return fullSteps.join('/');
  }
}

function parseImportExpression(
  typeAliasTypeElements: TypeAliasTypeEntity[], sourceFile: SourceFile, extraImport: string[]
): string {
  for (let i = 0; i < typeAliasTypeElements.length; i++) {
    const typeAliasTypeElement = typeAliasTypeElements[i];
    const typeName = typeAliasTypeElement.typeName;
    if (!typeName?.trim().startsWith('import(')) {
      continue;
    }
    const splitTypeName = typeName.split(')');
    const propertiesIndex = 1;
    const properties = splitTypeName[propertiesIndex];
    const importPath = getImportFileFullPath(typeName);
    const realImportPath = pathToImportPath(sourceFile.fileName, importPath);
    if (!importPath) {
      continue;
    }
    const importFileContent = fs.readFileSync(importPath, 'utf-8');
    if (properties.startsWith('.default')) {
      let result = importFileContent.match(/export\sdefault\sclass\s[a-zA-Z]+/);
      if (result) {
        const defaultModuleName = '_' + result[0].replace(/export\sdefault\sclass\s/, '');
        const importStr = `import ${defaultModuleName} from '${realImportPath}';\n`;
        !extraImport.includes(importStr) && extraImport.push(importStr);
        return `${defaultModuleName}${properties.replace('.default', '')}`;
      }
      result = importFileContent.match(/export\sdefault\s[a-zA-Z]+;/);
      if (result) {
        const moduleName = result[0]
          .replace(/export\sdefault\s/, '')
          .replace(';', '');
        const mockFunctionName = `mock${firstCharacterToUppercase(moduleName)}`;
        const importStr = `import {${mockFunctionName}} from '${realImportPath}';\n`;
        !extraImport.includes(importStr) && extraImport.push(importStr);
        return `${mockFunctionName}()${properties.replace('.default', '')}`;
      }
    } else {
      const moduleName = properties.replace('.', '').split('.')[0];
      const importStr = `import {${moduleName} as _${moduleName}} from '${realImportPath}';\n`;
      !extraImport.includes(importStr) && extraImport.push(importStr);
      return `_${properties.replace('.', '')}`;
    }
  }
  return '';
}
