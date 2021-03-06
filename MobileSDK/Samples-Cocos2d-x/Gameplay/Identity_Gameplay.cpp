// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <Identity_Integration.h>
#include <XSAPI_Integration.h>
#include <Game_Integration.h>
#include <GameScene.h>

#pragma region Identity Gameplay Internal
void Identity_Gameplay_CloseUserContext(_In_ XblContextHandle xblContext)
{
    if (xblContext)
    {
        XalUserHandle user = nullptr;
        HRESULT hr = XblContextGetUser(xblContext, &user);

        if (SUCCEEDED(hr))
        {
            XalUserCloseHandle(user);
        }

        XblContextCloseHandle(xblContext);
    }
}

HRESULT Identity_Gameplay_SignInUser(_In_ XalUserHandle newUser, _In_ bool resolveIssuesWithUI)
{
    // Call XalUserGetId here to ensure all vetos (gametag banned, etc) have passed
    uint64_t xuid = 0;
    HRESULT hr = XalUserGetId(newUser, &xuid);
    
    if (SUCCEEDED(hr))
    {
        XblContextHandle newXblContext = nullptr;
        hr = XblContextCreateHandle(newUser, &newXblContext);
        
        if (SUCCEEDED(hr))
        {
            // Close the previous Xbl Context, if one existed
            Identity_Gameplay_CloseUserContext(GameScene::getInstance()->getXblContext());
            
            GameScene::getInstance()->setXblContext(newXblContext);
            
            SampleLog(LL_INFO, ""); // New line
            if (resolveIssuesWithUI)
            {
                SampleLog(LL_INFO, "Sign-in with UI successful!");
            }
            else
            {
                SampleLog(LL_INFO, "Auto sign-in successful!");
            }
            
            std::string gamerTag;
            hr = Identity_GetGamerTag(newUser, &gamerTag);
            if (SUCCEEDED(hr))
            {
                SampleLog(LL_INFO, "Welcome %s!", gamerTag.c_str());
            }
            
            HubLayer* hubLayer = GameScene::getInstance()->m_hubLayer;
            if (hubLayer)
            {
                hubLayer->showLayer();
            }
        }
        else
        {
            SampleLog(LL_ERROR, "XblContextCreateHandle Failed!");
            SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
        }
    }
    else
    {
        SampleLog(LL_ERROR, "XalUserGetId Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
        
        if (resolveIssuesWithUI)
        {
            SampleLog(LL_INFO, ""); // New Line
            SampleLog(LL_INFO, "Trying to resolve user issue with UI");
            
            // Duplicate handle to prolong the user to be handled later by resolve
            XblUserHandle dupUser = nullptr;
            XalUserDuplicateHandle(newUser, &dupUser);
            // Note: Creates a Ref for XblUserHandle, will be closed inside XAL_Gameplay_TryResolveUserIssue
            
            HRESULT asyncResult = Identity_TryResolveUserIssue(XblGetAsyncQueue(), dupUser);
            
            if (FAILED(asyncResult))
            {
                SampleLog(LL_ERROR, "XalUserResolveIssueWithUiAsync Failed!");
                SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(asyncResult).c_str());
                
                if (dupUser) { XalUserCloseHandle(dupUser); }
            }
        }
    }
    
    return hr;
}

#pragma endregion

#pragma region Identity Gameplay
void Identity_Gameplay_TrySignInUserSilently(_In_ XalUserHandle newUser, _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game
    HRESULT hr = result;

    if (SUCCEEDED(result))
    {
        hr = Identity_Gameplay_SignInUser(newUser, false);
    }
    
    if (FAILED(hr))
    {
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "Auto sign-in failed! Please sign-in with the UI!");
    }

    GameScene::getInstance()->m_identityLayer->hasTriedSilentSignIn(true);
}

void Identity_Gameplay_TrySignInUserWithUI(_In_ XalUserHandle newUser, _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game
    HRESULT hr = result;
    
    if (SUCCEEDED(hr))
    {
        hr = Identity_Gameplay_SignInUser(newUser, true);
    }
    
    if (FAILED(hr))
    {
        SampleLog(LL_INFO, ""); // New Line
        SampleLog(LL_INFO, "Please try signing-in with UI again!");
        
        HubLayer* hubLayer = GameScene::getInstance()->m_hubLayer;
        if (hubLayer)
        {
            hubLayer->hideLayer();
        }
    }
}

void Identity_Gameplay_TryResolveUserIssue(_In_ XalUserHandle user, _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {
        SampleLog(LL_INFO, "Issues with sign-in resolved!");
    }
    else
    {
        SampleLog(LL_ERROR, "Issues with sign-in failed to resolve!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(result).c_str());
    }

    SampleLog(LL_INFO, ""); // New Line
    SampleLog(LL_INFO, "Please try signing-in with UI again!");

    // Close the Reference if one was created during XalUserDuplicateHandle
    if (user) { XalUserCloseHandle(user); }
}

void Identity_Gameplay_TrySignOutUser(_In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {
        XblContextHandle xblContext = GameScene::getInstance()->getXblContext();

        XalUserHandle user = nullptr;
        HRESULT hr = XblContextGetUser(xblContext, &user);

        if (SUCCEEDED(hr))
        {
            HubLayer* hubLayer = GameScene::getInstance()->m_hubLayer;

            if (hubLayer)
            {
                hubLayer->hideLayer();
            }

            std::string gamerTag;
            HRESULT hr = Identity_GetGamerTag(user, &gamerTag);
            if (SUCCEEDED(hr))
            {
                SampleLog(LL_INFO, "Goodbye %s!", gamerTag.c_str());
            }
        }

        Identity_Gameplay_CloseUserContext(xblContext);
        GameScene::getInstance()->setXblContext(nullptr);

        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "User sign-out successful!");
    }
    else
    {
        SampleLog(LL_INFO, ""); // New line
        SampleLog(LL_INFO, "User sign-out failed!");
    }
}

void Identity_Gameplay_GetDefaultGamerProfile(_In_ XblUserProfile userProfile, _In_ HRESULT result)
{
    HRESULT hr = result;
    
    if (SUCCEEDED(hr))
    {
        SampleLog(LL_INFO, "GamerTag: %s", userProfile.gamertag);
        SampleLog(LL_INFO, "Gamer Score: %s", userProfile.gamerscore);
    }
    
    if (FAILED(hr))
    {
        SampleLog(LL_INFO, "Get Default Gamer Profile failed!");
    }
}

void Identity_Gameplay_GetGamerProfile(_In_ XblUserProfile userProfile, _In_ HRESULT result)
{
    HRESULT hr = result;
    
    if (SUCCEEDED(hr))
    {
        // TODO: Add in functionality for Social Module
    }
    
    if (FAILED(hr))
    {
        SampleLog(LL_INFO, "Get Gamer Profile failed!");
    }
}

#pragma endregion
