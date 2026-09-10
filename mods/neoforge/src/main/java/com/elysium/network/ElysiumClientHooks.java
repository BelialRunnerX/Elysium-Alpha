package com.elysium.network;

import com.elysium.screen.ElysiumCharacterScreen;
import net.minecraft.client.Minecraft;

/**
 * The client half of the character packets, kept in its own class on purpose.
 *
 * Nothing here is referenced from a method reference or a field — only from
 * inside a lambda body in {@link ElysiumNetwork}. That is what keeps a
 * dedicated server from ever loading {@link Minecraft}: the lambda is created
 * during registration on both sides, but its body, and therefore this class,
 * is only resolved when a client-bound packet actually arrives.
 */
public final class ElysiumClientHooks {

    private ElysiumClientHooks() {
    }

    public static void openCharacterScreen(ElysiumPayloads.OpenCharacter payload) {
        Minecraft.getInstance().setScreen(new ElysiumCharacterScreen(
                CharacterSheet.parse(payload.sheet()), payload.unspent()));
    }
}
