#include "../Platform/stdafx.h"
#include "../Minecraft.h"
#include "../GameState/GameMode.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.player.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.storage.h"
#include "Input.h"
#include "../Player/LocalPlayer.h"
#include "../GameState/Options.h"

Input::Input() {
    xa = 0;
    ya = 0;
    wasJumping = false;
    jumping = false;
    sneaking = false;
    sprintKey = false;

    lReset = false;
    rReset = false;
}

void Input::tick(LocalPlayer* player) {
    // 4J Stu -  Assume that we only need one input class, even though the java
    // has subclasses for keyboard/controller This function is based on the
    // ControllerInput class in the Java, and will probably need changed
    // OutputDebugString("INPUT: Beginning input tick\n");

    Minecraft* pMinecraft = Minecraft::GetInstance();
    int iPad = player->GetXboxPad();

    // 4J-PB minecraft movement seems to be the wrong way round, so invert x!
    if (pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_LEFT) ||
        pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_RIGHT))
        xa = -InputManager.GetJoypadStick_LX(iPad);
    else
        xa = 0.0f;

    if (pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_FORWARD) ||
        pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_BACKWARD))
        ya = InputManager.GetJoypadStick_LY(iPad);
    else
        ya = 0.0f;

#ifndef _CONTENT_PACKAGE
    if (app.GetFreezePlayers()) {
        xa = ya = 0.0f;
        player->abilities.flying = true;
    }
#endif

    if (!lReset) {
        if (xa * xa + ya * ya == 0.0f) {
            lReset = true;
        }
        xa = ya = 0.0f;
    }

    // 4J - in flying mode, don't actually toggle sneaking
    if (!player->abilities.flying) {
        if ((player->ullButtonsPressed &
             (1LL << MINECRAFT_ACTION_SNEAK_TOGGLE)) &&
            pMinecraft->localgameModes[iPad]->isInputAllowed(
                MINECRAFT_ACTION_SNEAK_TOGGLE)) {
            sneaking = !sneaking;
        }
    }

    if (sneaking) {
        xa *= 0.3f;
        ya *= 0.3f;
    }

    float turnSpeed = 50.0f;

    float tx = 0.0f;
    float ty = 0.0f;
    if (pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_LOOK_LEFT) ||
        pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_LOOK_RIGHT))
        tx = InputManager.GetJoypadStick_RX(iPad) *
             (((float)app.GetGameSettings(iPad,
                                          eGameSetting_Sensitivity_InGame)) /
              100.0f);  // apply sensitivity to look
    if (pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_LOOK_UP) ||
        pMinecraft->localgameModes[iPad]->isInputAllowed(
            MINECRAFT_ACTION_LOOK_DOWN))
        ty = InputManager.GetJoypadStick_RY(iPad) *
             (((float)app.GetGameSettings(iPad,
                                          eGameSetting_Sensitivity_InGame)) /
              100.0f);  // apply sensitivity to look

#ifndef _CONTENT_PACKAGE
    if (app.GetFreezePlayers()) tx = ty = 0.0f;
#endif

    // 4J: WESTY : Invert look Y if required.
    if (app.GetGameSettings(iPad, eGameSetting_ControlInvertLook)) {
        ty = -ty;
    }

    if (!rReset) {
        if (tx * tx + ty * ty == 0.0f) {
            rReset = true;
        }
        tx = ty = 0.0f;
    }
    // The original console stick curve squares the look input, which is fine
    // for analog sticks but can make mouse-driven look on Linux feel like it
    // "sticks" around the current viewpoint.
    const float lookDeadzone = 0.01f;
    if (Mth::abs(tx) < lookDeadzone) tx = 0.0f;
    if (Mth::abs(ty) < lookDeadzone) ty = 0.0f;

#ifdef __linux__
    // Keep look movement responsive for mouse/desktop input.
    player->interpolateTurn(tx * turnSpeed, ty * turnSpeed);
#else
    player->interpolateTurn(tx * abs(tx) * turnSpeed,
                            ty * abs(ty) * turnSpeed);
#endif

    // jumping = controller.isButtonPressed(0);

    sprintKey = InputManager.GetValue(iPad, MINECRAFT_ACTION_SPRINT) &&
                pMinecraft->localgameModes[iPad]->isInputAllowed(
                    MINECRAFT_ACTION_SPRINT);
    jumping =
        InputManager.GetValue(iPad, MINECRAFT_ACTION_JUMP) &&
        pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_JUMP);

#ifndef _CONTENT_PACKAGE
    if (app.GetFreezePlayers()) jumping = false;
#endif

    // OutputDebugString("INPUT: End input tick\n");
}
