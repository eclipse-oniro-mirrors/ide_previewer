/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

/**
 * @file
 * @kit AbilityKit
 */

import { AsyncCallback, Callback } from './@ohos.base';
import { Permissions } from './permissions';
import type _Context from './application/Context';
import type _PermissionRequestResult from './security/PermissionRequestResult';

/**
 * @namespace abilityAccessCtrl
 * @syscap SystemCapability.Security.AccessToken
 * @since 8
 */
/**
 * @namespace abilityAccessCtrl
 * @syscap SystemCapability.Security.AccessToken
 * @atomicservice
 * @since 11
 */
declare namespace abilityAccessCtrl {
  /**
   * Obtains the AtManager instance.
   *
   * @returns { AtManager } Returns the instance of the AtManager.
   * @syscap SystemCapability.Security.AccessToken
   * @since 8
   */
  /**
   * Obtains the AtManager instance.
   *
   * @returns { AtManager } returns the instance of the AtManager.
   * @syscap SystemCapability.Security.AccessToken
   * @crossplatform
   * @since 10
   */
  /**
   * Obtains the AtManager instance.
   *
   * @returns { AtManager } returns the instance of the AtManager.
   * @syscap SystemCapability.Security.AccessToken
   * @crossplatform
   * @atomicservice
   * @since 11
   */
  function createAtManager(): AtManager;

  /**
   * Provides methods for managing access_token.
   *
   * @interface AtManager
   * @syscap SystemCapability.Security.AccessToken
   * @since 8
   */
  /**
   * Provides methods for managing access_token.
   *
   * @interface AtManager
   * @syscap SystemCapability.Security.AccessToken
   * @atomicservice
   * @since 11
   */
  interface AtManager {
    /**
     * Checks whether a specified application has been granted the given permission.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be verified. The Permissions type supports only valid permission names.
     * @returns { Promise<GrantStatus> } Returns permission verify result.
     * @syscap SystemCapability.Security.AccessToken
     * @since 9
     */
    verifyAccessToken(tokenID: number, permissionName: Permissions): Promise<GrantStatus>;

    /**
     * Checks whether a specified application has been granted the given permission.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { string } permissionName - Name of the permission to be verified.
     * @returns { Promise<GrantStatus> } Returns permission verify result.
     * @syscap SystemCapability.Security.AccessToken
     * @since 8
     * @deprecated since 9
     * @useinstead ohos.abilityAccessCtrl.AtManager#checkAccessToken
     */
    verifyAccessToken(tokenID: number, permissionName: string): Promise<GrantStatus>;

    /**
     * Checks whether a specified application has been granted the given permission synchronously.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be verified.
     * @returns { GrantStatus } Returns permission verify result.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @syscap SystemCapability.Security.AccessToken
     * @since 9
     */
    verifyAccessTokenSync(tokenID: number, permissionName: Permissions): GrantStatus;

    /**
     * Checks whether a specified application has been granted the given permission.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be verified.
     * @returns { Promise<GrantStatus> } Returns permission verify result.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @syscap SystemCapability.Security.AccessToken
     * @since 9
     */
    /**
     * Checks whether a specified application has been granted the given permission.
     * On the cross-platform, this function can be used to check the permission grant status for the current application only.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be verified.
     * @returns { Promise<GrantStatus> } Returns permission verify result.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @since 10
     */
    /**
     * Checks whether a specified application has been granted the given permission.
     * On the cross-platform, this function can be used to check the permission grant status for the current application only.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be verified.
     * @returns { Promise<GrantStatus> } Returns permission verify result.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    checkAccessToken(tokenID: number, permissionName: Permissions): Promise<GrantStatus>;

    /**
     * Checks whether a specified application has been granted the given permission.
     * On the cross-platform, this function can be used to check the permission grant status for the current application only.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be verified.
     * @returns { GrantStatus } Returns permission verify result.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @since 10
     */
    /**
     * Checks whether a specified application has been granted the given permission.
     * On the cross-platform, this function can be used to check the permission grant status for the current application only.
     *
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be verified.
     * @returns { GrantStatus } Returns permission verify result.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    checkAccessTokenSync(tokenID: number, permissionName: Permissions): GrantStatus;

    /**
     * Requests certain permissions from the user.
     *
     * @param { Context } context - The context that initiates the permission request.
     * @param { Array<Permissions> } permissionList - Indicates the list of permissions to be requested. This parameter cannot be null or empty.
     * @param { AsyncCallback<PermissionRequestResult> } requestCallback Callback for the result from requesting permissions.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The context is invalid when it does not belong to the application itself.
     * @syscap SystemCapability.Security.AccessToken
     * @stagemodelonly
     * @since 9
     */
    /**
     * Requests certain permissions from the user.
     *
     * @param { Context } context - The context that initiates the permission request.
     * @param { Array<Permissions> } permissionList - Indicates the list of permissions to be requested. This parameter cannot be null or empty.
     * @param { AsyncCallback<PermissionRequestResult> } requestCallback Callback for the result from requesting permissions.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The context is invalid when it does not belong to the application itself.
     * @syscap SystemCapability.Security.AccessToken
     * @stagemodelonly
     * @crossplatform
     * @since 10
     */
    requestPermissionsFromUser(
      context: Context,
      permissionList: Array<Permissions>,
      requestCallback: AsyncCallback<PermissionRequestResult>
    ): void;

    /**
     * Requests certain permissions from the user.
     *
     * @param { Context } context - The context that initiates the permission request.
     * @param { Array<Permissions> } permissionList - Indicates the list of permissions to be requested. This parameter cannot be null or empty.
     * @returns { Promise<PermissionRequestResult> } Returns result of requesting permissions.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The context is invalid when it does not belong to the application itself.
     * @syscap SystemCapability.Security.AccessToken
     * @stagemodelonly
     * @since 9
     */
    /**
     * Requests certain permissions from the user.
     *
     * @param { Context } context - The context that initiates the permission request.
     * @param { Array<Permissions> } permissionList - Indicates the list of permissions to be requested. This parameter cannot be null or empty.
     * @returns { Promise<PermissionRequestResult> } Returns result of requesting permissions.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The context is invalid when it does not belong to the application itself.
     * @syscap SystemCapability.Security.AccessToken
     * @stagemodelonly
     * @crossplatform
     * @since 10
     */
    /**
     * Requests certain permissions from the user.
     *
     * @param { Context } context - The context that initiates the permission request.
     * @param { Array<Permissions> } permissionList - Indicates the list of permissions to be requested. This parameter cannot be null or empty.
     * @returns { Promise<PermissionRequestResult> } Returns result of requesting permissions.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The context is invalid when it does not belong to the application itself.
     * @syscap SystemCapability.Security.AccessToken
     * @stagemodelonly
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    requestPermissionsFromUser(context: Context, permissionList: Array<Permissions>): Promise<PermissionRequestResult>;

    /**
     * Grants a specified user_grant permission to the given application.
     *
     * @permission ohos.permission.GRANT_SENSITIVE_PERMISSIONS
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be granted.
     * @param { number } permissionFlags - Flags of permission state.
     * @returns { Promise<void> } The promise returned by the function.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission "ohos.permission.GRANT_SENSITIVE_PERMISSIONS".
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256,
     *  or the flags value is invalid.
     * @throws { BusinessError } 12100002 - The specified tokenID does not exist.
     * @throws { BusinessError } 12100003 - The specified permission does not exist.
     * @throws { BusinessError } 12100006 - The application specified by the tokenID is not allowed to be granted with the specified permission.
     *  Either the application is a sandbox or the tokenID is from a remote device.
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 8
     */
    grantUserGrantedPermission(tokenID: number, permissionName: Permissions, permissionFlags: number): Promise<void>;

    /**
     * Grants a specified user_grant permission to the given application.
     *
     * @permission ohos.permission.GRANT_SENSITIVE_PERMISSIONS
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be granted.
     * @param { number } permissionFlags - Flags of permission state.
     * @param { AsyncCallback<void> } callback - Asynchronous callback interface.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission "ohos.permission.GRANT_SENSITIVE_PERMISSIONS".
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256,
     *  or the flags value is invalid.
     * @throws { BusinessError } 12100002 - The specified tokenID does not exist.
     * @throws { BusinessError } 12100003 - The specified permission does not exist.
     * @throws { BusinessError } 12100006 - The application specified by the tokenID is not allowed to be granted with the specified permission.
     *  Either the application is a sandbox or the tokenID is from a remote device.
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 8
     */
    grantUserGrantedPermission(
      tokenID: number,
      permissionName: Permissions,
      permissionFlags: number,
      callback: AsyncCallback<void>
    ): void;

    /**
     * Revoke a specified user_grant permission to the given application.
     *
     * @permission ohos.permission.REVOKE_SENSITIVE_PERMISSIONS
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be revoked.
     * @param { number } permissionFlags - Flags of permission state.
     * @returns { Promise<void> } The promise returned by the function.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission "ohos.permission.REVOKE_SENSITIVE_PERMISSIONS".
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256,
     *  or the flags value is invalid.
     * @throws { BusinessError } 12100002 - The specified tokenID does not exist.
     * @throws { BusinessError } 12100003 - The specified permission does not exist.
     * @throws { BusinessError } 12100006 - The application specified by the tokenID is not allowed to be revoked with the specified permission.
     *  Either the application is a sandbox or the tokenID is from a remote device.
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 8
     */
    revokeUserGrantedPermission(tokenID: number, permissionName: Permissions, permissionFlags: number): Promise<void>;

    /**
     * Revoke a specified user_grant permission to the given application.
     *
     * @permission ohos.permission.REVOKE_SENSITIVE_PERMISSIONS
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be revoked.
     * @param { number } permissionFlags - Flags of permission state.
     * @param { AsyncCallback<void> } callback - Asynchronous callback interface.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission "ohos.permission.REVOKE_SENSITIVE_PERMISSIONS".
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256,
     *  or the flags value is invalid.
     * @throws { BusinessError } 12100002 - The specified tokenID does not exist.
     * @throws { BusinessError } 12100003 - The specified permission does not exist.
     * @throws { BusinessError } 12100006 - The application specified by the tokenID is not allowed to be revoked with the specified permission.
     *  Either the application is a sandbox or the tokenID is from a remote device.
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 8
     */
    revokeUserGrantedPermission(
      tokenID: number,
      permissionName: Permissions,
      permissionFlags: number,
      callback: AsyncCallback<void>
    ): void;

    /**
     * Queries specified permission flags of the given application.
     *
     * @permission ohos.permission.GET_SENSITIVE_PERMISSIONS or ohos.permission.GRANT_SENSITIVE_PERMISSIONS or ohos.permission.REVOKE_SENSITIVE_PERMISSIONS
     * @param { number } tokenID - Token ID of the application.
     * @param { Permissions } permissionName - Name of the permission to be get.
     * @returns { Promise<number> } Return permission flags.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission specified below.
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @throws { BusinessError } 12100002 - The specified tokenID does not exist.
     * @throws { BusinessError } 12100003 - The specified permission does not exist.
     * @throws { BusinessError } 12100006 - The operation is not allowed. Either the application is a sandbox or the tokenID is from a remote device.
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 8
     */
    getPermissionFlags(tokenID: number, permissionName: Permissions): Promise<number>;

    /**
     * Queries permission management version.
     *
     * @returns { Promise<number> } Return permission version.
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    getVersion(): Promise<number>;

    /**
     * Queries permissions status of the given application.
     *
     * @permission ohos.permission.GET_SENSITIVE_PERMISSIONS
     * @param { number } tokenID - Token ID of the application.
     * @param { Array<Permissions> } permissionList - Indicates the list of permissions to be queried. This parameter cannot be null or empty.
     * @returns { Promise<Array<PermissionStatus>> } Return permission status.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission "ohos.permission.GET_SENSITIVE_PERMISSIONS".
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the permissionList is empty.
     * @throws { BusinessError } 12100002 - The specified tokenID does not exist.
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 12
     */
    getPermissionsStatus(tokenID: number, permissionList: Array<Permissions>): Promise<Array<PermissionStatus>>;

    /**
     * Registers a permission state callback so that the application can be notified upon specified permission state of specified applications changes.
     *
     * @permission ohos.permission.GET_SENSITIVE_PERMISSIONS
     * @param { 'permissionStateChange' } type - Event type.
     * @param { Array<number> } tokenIDList - A list of permissions that specify the permissions to be listened on. The value in the list can be:
     *        <ul>
     *        <li>{@code empty} - Indicates that the application can be notified if the specified permission state of any applications changes.
     *        </li>
     *        <li>{@code non-empty} - Indicates that the application can only be notified if the specified
     *                                permission state of the specified applications change.
     *        </li>
     *        </ul>
     * @param { Array<Permissions> } permissionList - A list of permissions that specify the permissions to be listened on. The value in the list can be:
     *        <ul>
     *        <li>{@code empty} - Indicates that the application can be notified if any permission state of the specified applications changes.
     *        </li>
     *        <li>{@code non-empty} - Indicates that the application can only be notified if the specified
     *                                permission state of the specified applications changes.
     *        </li>
     *        </ul>
     * @param { Callback<PermissionStateChangeInfo> } callback - Callback for the result from registering permissions.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission "ohos.permission.GET_SENSITIVE_PERMISSIONS".
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenID is 0, or the string size of permissionName is larger than 256.
     * @throws { BusinessError } 12100004 - The interface is called repeatedly with the same input.
     * @throws { BusinessError } 12100005 - The registration time has exceeded the limitation.
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @throws { BusinessError } 12100008 - Out of memory.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    on(
      type: 'permissionStateChange',
      tokenIDList: Array<number>,
      permissionList: Array<Permissions>,
      callback: Callback<PermissionStateChangeInfo>
    ): void;

    /**
     * Unregisters a permission state callback so that the specified applications cannot be notified upon specified permissions state changes anymore.
     *
     * @permission ohos.permission.GET_SENSITIVE_PERMISSIONS
     * @param { 'permissionStateChange' } type - Event type.
     * @param { Array<number> } tokenIDList - A list of permissions that specify the permissions to be listened on.
     *  It should correspond to the value registered by function of "on", whose type is "permissionStateChange".
     * @param { Array<Permissions> } permissionList - A list of permissions that specify the permissions to be listened on.
     *  It should correspond to the value registered by function of "on", whose type is "permissionStateChange".
     * @param { Callback<PermissionStateChangeInfo> } callback - Callback for the result from unregistering permissions.
     * @throws { BusinessError } 401 - The parameter check failed.
     * @throws { BusinessError } 201 - Permission denied. Interface caller does not have permission "ohos.permission.GET_SENSITIVE_PERMISSIONS".
     * @throws { BusinessError } 202 - Not System App. Interface caller is not a system app.
     * @throws { BusinessError } 12100001 - The parameter is invalid. The tokenIDs or permissionNames in the list are all invalid.
     * @throws { BusinessError } 12100004 - The interface is not used together with "on".
     * @throws { BusinessError } 12100007 - Service is abnormal.
     * @throws { BusinessError } 12100008 - Out of memory.
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    off(
      type: 'permissionStateChange',
      tokenIDList: Array<number>,
      permissionList: Array<Permissions>,
      callback?: Callback<PermissionStateChangeInfo>
    ): void;
  }

  /**
   * GrantStatus.
   *
   * @enum { number }
   * @syscap SystemCapability.Security.AccessToken
   * @since 8
   */
  /**
   * GrantStatus.
   *
   * @enum { number }
   * @syscap SystemCapability.Security.AccessToken
   * @crossplatform
   * @since 10
   */
  /**
   * GrantStatus.
   *
   * @enum { number }
   * @syscap SystemCapability.Security.AccessToken
   * @crossplatform
   * @atomicservice
   * @since 11
   */
  export enum GrantStatus {
    /**
     * access_token permission check fail
     *
     * @syscap SystemCapability.Security.AccessToken
     * @since 8
     */
    /**
     * access_token permission check fail
     *
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @since 10
     */
    /**
     * access_token permission check fail
     *
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    PERMISSION_DENIED = -1,
    /**
     * access_token permission check success
     *
     * @syscap SystemCapability.Security.AccessToken
     * @since 8
     */
    /**
     * access_token permission check success
     *
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @since 10
     */
    /**
     * access_token permission check success
     *
     * @syscap SystemCapability.Security.AccessToken
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    PERMISSION_GRANTED = 0
  }

  /**
   * Enum for permission state change type.
   *
   * @enum { number }
   * @syscap SystemCapability.Security.AccessToken
   * @systemapi
   * @since 9
   */
  export enum PermissionStateChangeType {
    /**
     * A granted user_grant permission is revoked.
     *
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    PERMISSION_REVOKED_OPER = 0,
    /**
     * A user_grant permission is granted.
     *
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    PERMISSION_GRANTED_OPER = 1
  }

  /**
   * Indicates the information of permission state change.
   *
   * @interface PermissionStateChangeInfo
   * @syscap SystemCapability.Security.AccessToken
   * @systemapi
   * @since 9
   * @name PermissionStateChangeInfo
   */
  interface PermissionStateChangeInfo {
    /**
     * Indicates the permission state change type.
     *
     * @type { PermissionStateChangeType }
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    change: PermissionStateChangeType;

    /**
     * Indicates the application whose permission state has been changed.
     *
     * @type { number }
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    tokenID: number;

    /**
     * Indicates the permission whose state has been changed.
     *
     * @type { Permissions }
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 9
     */
    permissionName: Permissions;
  }

  /**
   * PermissionStatus.
   *
   * @enum { number }
   * @syscap SystemCapability.Security.AccessToken
   * @systemapi
   * @since 12
   */
  export enum PermissionStatus {
    /**
     * permission has been denied, only can change it in settings
     *
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 12
     */
    DENIED = -1,
    /**
     * permission has been granted
     *
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 12
     */
    GRANTED = 0,
    /**
     * permission is not determined
     *
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 12
     */
    NOT_DETERMINED = 1,
    /**
     * permission is invalid
     *
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 12
     */
    INVALID = 2,
    /**
     * permission has been restricted
     *
     * @syscap SystemCapability.Security.AccessToken
     * @systemapi
     * @since 12
     */
    RESTRICTED = 3
  }
}

export default abilityAccessCtrl;
export { Permissions };
/**
 * PermissionRequestResult interface.
 *
 * @syscap SystemCapability.Security.AccessToken
 * @stagemodelonly
 * @crossplatform
 * @since 10
 */
/**
 * PermissionRequestResult interface.
 *
 * @syscap SystemCapability.Security.AccessToken
 * @stagemodelonly
 * @crossplatform
 * @atomicservice
 * @since 11
 */
export type PermissionRequestResult = _PermissionRequestResult;
/**
 * Context interface.
 *
 * @syscap SystemCapability.Security.AccessToken
 * @stagemodelonly
 * @crossplatform
 * @since 10
 */
/**
 * Context interface.
 *
 * @syscap SystemCapability.Security.AccessToken
 * @stagemodelonly
 * @crossplatform
 * @atomicservice
 * @since 11
 */
export type Context = _Context;
