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

import type { ClassDeclaration, SourceFile } from 'typescript';
import {
  isConstructorDeclaration,
  isGetAccessor,
  isMethodDeclaration,
  isPropertyDeclaration,
  isTypeParameterDeclaration,
  SyntaxKind
} from 'typescript';
import { getExportKeyword } from '../common/commonUtils';
import { getConstructorDeclaration } from './constructorDeclaration';
import type { ConstructorEntity } from './constructorDeclaration';
import { getHeritageClauseDeclaration } from './heritageClauseDeclaration';
import type { HeritageClauseEntity } from './heritageClauseDeclaration';
import { getMethodDeclaration } from './methodDeclaration';
import type { MethodEntity, StaticMethodEntity } from './methodDeclaration';
import { getGetDeclaration, getPropertyDeclaration } from './propertyDeclaration';
import type { PropertyEntity } from './propertyDeclaration';
import { getTypeParameterDeclaration } from './typeParameterDeclaration';
import type { TypeParameterEntity } from './typeParameterDeclaration';

interface SubstepClassEntity {
  className: string;
  typeParameters: Array<TypeParameterEntity>;
  classConstructor: Array<Array<ConstructorEntity>>;
  classMethod: Map<string, Array<MethodEntity>>;
  classProperty: Array<PropertyEntity>;
  staticMethods: Array<StaticMethodEntity>;
}

export interface ClassEntity {
  className: string;
  typeParameters: Array<TypeParameterEntity>;
  heritageClauses: Array<HeritageClauseEntity>;
  classConstructor: Array<Array<ConstructorEntity>>;
  classMethod: Map<string, Array<MethodEntity>>;
  classProperty: Array<PropertyEntity>;
  exportModifiers: Array<number>;
  staticMethods: Array<StaticMethodEntity>;
}

/**
 * get class info
 * @param classNode
 * @param sourceFile
 * @returns
 */
export function getClassDeclaration(classNode: ClassDeclaration, sourceFile: SourceFile): ClassEntity {
  let exportModifiers: Array<number> = [];
  if (classNode.modifiers !== undefined) {
    exportModifiers = getExportKeyword(classNode.modifiers);
  }

  const heritageClauses: Array<HeritageClauseEntity> = [];

  if (classNode.heritageClauses !== undefined) {
    classNode.heritageClauses.forEach(value => {
      heritageClauses.push(getHeritageClauseDeclaration(value, sourceFile));
    });
  }

  const substepClassEntitys: SubstepClassEntity = substepGetClass(classNode, sourceFile);
  return {
    ...substepClassEntitys,
    exportModifiers,
    heritageClauses
  };
}

/**
 *get some class info
 * @param classNode
 * @param sourceFile
 * @returns
 */
function substepGetClass(classNode: ClassDeclaration, sourceFile: SourceFile): SubstepClassEntity {
  const className = classNode.name === undefined ? '' : classNode.name.escapedText.toString();
  const classConstructor: Array<Array<ConstructorEntity>> = [];
  const classMethod: Map<string, Array<MethodEntity>> = new Map<string, Array<MethodEntity>>();
  const classProperty: Array<PropertyEntity> = [];
  const typeParameters: Array<TypeParameterEntity> = [];
  const staticMethods: Array<StaticMethodEntity> = [];
  classNode.members.forEach(value => {
    if (isMethodDeclaration(value)) {
      const methodEntity = getMethodDeclaration(value, sourceFile);
      if (methodEntity.modifiers.includes(SyntaxKind.StaticKeyword)) {
        staticMethods.push({ className: className, methodEntity: methodEntity });
      } else {
        if (classMethod.get(methodEntity.functionName.name) !== undefined) {
          classMethod.get(methodEntity.functionName.name)?.push(methodEntity);
        } else {
          const methodArray: Array<MethodEntity> = [];
          methodArray.push(methodEntity);
          classMethod.set(methodEntity.functionName.name, methodArray);
        }
      }
    } else if (isPropertyDeclaration(value)) {
      classProperty.push(getPropertyDeclaration(value, sourceFile));
    } else if (isConstructorDeclaration(value)) {
      classConstructor.push(getConstructorDeclaration(value, sourceFile));
    } else if (isTypeParameterDeclaration(value)) {
      typeParameters.push(getTypeParameterDeclaration(value, sourceFile));
    } else if (isGetAccessor(value)) {
      classProperty.push(getGetDeclaration(value, sourceFile));
    } else {
      console.log('--------------------------- uncaught class type start -----------------------');
      console.log('className: ' + className);
      console.log(value);
      console.log('--------------------------- uncaught class type end -----------------------');
    }
  });
  return {
    className,
    typeParameters,
    classConstructor,
    classMethod,
    classProperty,
    staticMethods
  };
}
