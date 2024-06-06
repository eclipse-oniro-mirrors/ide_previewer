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

import {
  isCallSignatureDeclaration, isConstructSignatureDeclaration, isHeritageClause, isIndexSignatureDeclaration,
  isMethodSignature, isPropertySignature, isTypeParameterDeclaration
} from 'typescript';
import type { InterfaceDeclaration, SourceFile } from 'typescript';
import { getExportKeyword } from '../common/commonUtils';
import { getCallSignatureDeclaration } from './callSignatureDeclaration';
import type { CallSignatureEntity } from './callSignatureDeclaration';
import { getConstructorDeclaration } from './constructorDeclaration';
import type { ConstructorEntity } from './constructorDeclaration';
import { getHeritageClauseDeclaration } from './heritageClauseDeclaration';
import type { HeritageClauseEntity } from './heritageClauseDeclaration';
import { getIndexSignatureDeclaration } from './indexSignatureDeclaration';
import type { IndexSignatureEntity } from './indexSignatureDeclaration';
import { getMethodSignatureDeclaration } from './methodSignatureDeclaration';
import type { MethodSignatureEntity } from './methodSignatureDeclaration';
import { getPropertySignatureDeclaration } from './propertySignatureDeclaration';
import type { PropertySignatureEntity } from './propertySignatureDeclaration';
import { getTypeParameterDeclaration } from './typeParameterDeclaration';
import type { TypeParameterEntity } from './typeParameterDeclaration';

interface SubstepInterfaceEntity {
  interfaceName: string,
  heritageClauses: Array<HeritageClauseEntity>,
  typeParameters: Array<TypeParameterEntity>,
  interfaceConstructors: Array<Array<ConstructorEntity>>,
  interfaceMethodSignature: Map<string, Array<MethodSignatureEntity>>,
  interfacePropertySignatures: Array<PropertySignatureEntity>,
  callSignatures: Array<CallSignatureEntity>,
  indexSignature: Array<IndexSignatureEntity>
}

interface SubstepGetClassparam {
  interfaceNode: InterfaceDeclaration,
  sourceFile: SourceFile,
  heritageClauses: Array<HeritageClauseEntity>
}
export interface InterfaceEntity {
  interfaceName: string,
  typeParameters: Array<TypeParameterEntity>,
  heritageClauses: Array<HeritageClauseEntity>,
  interfaceConstructors: Array<Array<ConstructorEntity>>,
  interfaceMethodSignature: Map<string, Array<MethodSignatureEntity>>,
  interfacePropertySignatures: Array<PropertySignatureEntity>,
  callSignatures: Array<CallSignatureEntity>,
  exportModifiers: Array<number>,
  indexSignature: Array<IndexSignatureEntity>
}

/**
 * get interface info
 * @param interfaceNode
 * @param sourceFile
 * @returns
 */
export function getInterfaceDeclaration(interfaceNode: InterfaceDeclaration, sourceFile: SourceFile): InterfaceEntity {
  let exportModifiers: Array<number> = [];
  if (interfaceNode.modifiers !== undefined) {
    exportModifiers = getExportKeyword(interfaceNode.modifiers);
  }

  const heritageClauses: Array<HeritageClauseEntity> = [];

  if (interfaceNode.heritageClauses !== undefined) {
    interfaceNode.heritageClauses.forEach(value => {
      heritageClauses.push(getHeritageClauseDeclaration(value, sourceFile));
    });
  }
  const SubstepInterfaceEntitys: SubstepInterfaceEntity = substepGetInterface({
    interfaceNode,
    sourceFile,
    heritageClauses
  });

  return {
    ...SubstepInterfaceEntitys,
    exportModifiers
  };
}

/**
 * get some interface info
 * @param substepGetClassparam
 * @returns
 */
function substepGetInterface(substepGetClassparam: SubstepGetClassparam): SubstepInterfaceEntity {
  const { interfaceNode, sourceFile, heritageClauses } = substepGetClassparam;
  const interfaceName = interfaceNode.name.escapedText.toString();
  const interfaceConstructors: Array<Array<ConstructorEntity>> = [];
  const interfaceMethodSignature: Map<string, Array<MethodSignatureEntity>> = new Map<string, Array<MethodSignatureEntity>>();
  const interfacePropertySignatures: Array<PropertySignatureEntity> = [];
  const callSignature: Array<CallSignatureEntity> = [];
  const typeParameters: Array<TypeParameterEntity> = [];
  const indexSignature: Array<IndexSignatureEntity> = [];
  interfaceNode.members.forEach(value => {
    if (isPropertySignature(value)) {
      interfacePropertySignatures.push(getPropertySignatureDeclaration(value, sourceFile));
    } else if (isMethodSignature(value)) {
      const methodSignature = getMethodSignatureDeclaration(value, sourceFile);
      if (interfaceMethodSignature.get(methodSignature.functionName) !== undefined) {
        interfaceMethodSignature.get(methodSignature.functionName)?.push(methodSignature);
      } else {
        const methodSignatureArray: Array<MethodSignatureEntity> = [];
        methodSignatureArray.push(methodSignature);
        interfaceMethodSignature.set(methodSignature.functionName, methodSignatureArray);
      }
    } else if (isHeritageClause(value)) {
      heritageClauses.push(getHeritageClauseDeclaration(value, sourceFile));
    } else if (isConstructSignatureDeclaration(value)) {
      interfaceConstructors.push(getConstructorDeclaration(value, sourceFile));
    } else if (isCallSignatureDeclaration(value)) {
      callSignature.push(getCallSignatureDeclaration(value, sourceFile));
    } else if (isTypeParameterDeclaration(value)) {
      typeParameters.push(getTypeParameterDeclaration(value, sourceFile));
    } else if (isIndexSignatureDeclaration(value)) {
      indexSignature.push(getIndexSignatureDeclaration(value, sourceFile));
    } else {
      console.log('--------------------------- uncaught interface type start -----------------------');
      console.log('interfaceName: ' + interfaceName);
      console.log(value);
      console.log('--------------------------- uncaught interface type end -----------------------');
    }
  });
  return {
    interfaceName,
    typeParameters,
    heritageClauses,
    interfaceConstructors,
    interfaceMethodSignature,
    interfacePropertySignatures,
    callSignatures: callSignature,
    indexSignature
  };
}
