//--------------------------------------------------------------------------------------
// ErrorPopUpScreen.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "ErrorPopUpScreen.h"

#include "InputState.h"
#include "ScreenManager.h"

using namespace DirectX;

namespace
{
    const XMVECTORF32 BackgroundColor = Colors::DarkSlateGray;
}

namespace GameSaveSample {

ErrorPopUpScreen::ErrorPopUpScreen(const std::shared_ptr<ScreenManager>& screenManager, winrt::hstring const & message) : GameScreen(screenManager),
    m_backgroundTexture(nullptr),
    m_errorMessage(message)
{
    m_exitWhenHidden = false;
    m_isPopup = true;
}

ErrorPopUpScreen::~ErrorPopUpScreen(void)
{
    UnloadContent();
}

void ErrorPopUpScreen::LoadContent()
{
    auto screenManager = this->Manager();
    m_backgroundTexture = screenManager->GetContentManager()->LoadTexture(L"Assets\\blank.png");
}

void ErrorPopUpScreen::UnloadContent()
{
    m_backgroundTexture = nullptr;
}

void ErrorPopUpScreen::HandleInput(const DirectX::InputState& inputState)
{
    if (inputState.IsMenuSelect(-1, nullptr))
        ExitScreen();
}

void ErrorPopUpScreen::Draw(float totalTime, float elapsedTime)
{
    UNREFERENCED_PARAMETER(totalTime);
    UNREFERENCED_PARAMETER(elapsedTime);

    auto screenManager = Manager();
    auto spriteBatch = screenManager->GetSpriteBatch();
    auto spriteFont = screenManager->GetSpriteFont();
    auto blendStates = screenManager->GetCommonStates();
    auto viewportBounds = screenManager->GetScreenBounds();
    float viewportWidth = float(viewportBounds.right);
    float viewportHeight = float(viewportBounds.bottom);
    auto scaleMatrix = DX::GetScaleMatrixForWindow(screenManager->GetWindowBounds());

    // calculate position and size of error message
    XMFLOAT2 errorMsgPosition = XMFLOAT2(0, viewportHeight / 2.0f);
    XMVECTORF32 errorMsgColor = Colors::Yellow;
    XMFLOAT2 origin = XMFLOAT2(0, spriteFont->GetLineSpacing() / 2.0f);
    XMVECTOR size = spriteFont->MeasureString(m_errorMessage.c_str());
    errorMsgPosition.x = viewportWidth / 2.0f - XMVectorGetX(size) / 2.0f;

    // create a rectangle representing the screen dimensions of the error message background rectangle
    long rectangleWidth = long(std::min(std::max(XMVectorGetX(size) + 100.0f, 600.0f), viewportWidth));
    long rectangleHeight = long(spriteFont->GetLineSpacing() * 6.0f);
    long rectangleLeft = long(viewportWidth / 2.0f) - (rectangleWidth / 2);
    long rectangleTop = long(errorMsgPosition.y + spriteFont->GetLineSpacing()) - (rectangleHeight / 2);
    RECT backgroundRectangle = { rectangleLeft, rectangleTop, rectangleLeft + rectangleWidth, rectangleTop + rectangleHeight };

    spriteBatch->Begin(SpriteSortMode_Deferred, blendStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);

    // draw a background color for the rectangle
    spriteBatch->Draw(m_backgroundTexture->GetResourceViewTemporary(), backgroundRectangle, BackgroundColor);

    // draw error message in the middle of the screen
    spriteFont->DrawString(spriteBatch.get(), m_errorMessage.c_str(), errorMsgPosition, errorMsgColor, 0, origin);

    // draw continuation prompt
    winrt::hstring continuePrompt = L"Press (A) to Continue";
    if (!InputState::IsAnyGamepadConnected())
    {
        continuePrompt = L"Press Enter to Continue";
    }
    errorMsgPosition.y += spriteFont->GetLineSpacing();
    size = spriteFont->MeasureString(continuePrompt.c_str());
    errorMsgPosition.x = viewportWidth / 2.0f - XMVectorGetX(size) / 2.0f;
    spriteFont->DrawString(spriteBatch.get(), continuePrompt.c_str(), errorMsgPosition, Colors::Yellow, 0, origin);

    spriteBatch->End();
}
} // namespace GameSaveSample
