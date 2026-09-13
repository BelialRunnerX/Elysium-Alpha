package com.elysium.event;

import com.elysium.Elysium;
import com.elysium.character.ElysiumCharacter;
import com.elysium.item.ElysiumCodexItem;
import com.elysium.network.ElysiumNetwork;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.ItemStack;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.neoforge.event.entity.player.PlayerEvent;

/**
 * First join: hand over a Codex and open the choice screen.
 *
 * <h2>Why the screen is pushed rather than pulled</h2>
 *
 * The client cannot know whether a character has been created — attachments
 * live on the server. So the server decides, and sends. That also means a
 * player joining with the mod installed but no character always gets the
 * screen, however they arrived: new world, new server, restored backup, or a
 * save from before this system existed.
 *
 * <h2>Why nothing is forced beyond the screen</h2>
 *
 * The screen refuses Escape until a choice is made, but the player is not
 * frozen and the world is not paused. Somebody who really wants to walk around
 * as an unchosen character can close the game and come back to the same
 * screen; they simply get no stats until they answer. Trapping a player in a
 * modal on a server they just joined is a worse failure than an unchosen
 * character walking three steps.
 */
@EventBusSubscriber(modid = Elysium.MODID)
public final class ElysiumJoinHandler {

    private ElysiumJoinHandler() {
    }

    @SubscribeEvent
    public static void onLoggedIn(PlayerEvent.PlayerLoggedInEvent event) {
        Player player = event.getEntity();
        if (player.level().isClientSide()) {
            return;
        }

        if (ElysiumCodexItem.shouldGrant(player)) {
            ElysiumCodexItem.grant(player, new ItemStack(Elysium.IMPERIAL_CODEX.get()));
            ElysiumCharacter.markCodexGiven(player);
        }

        // Reopened on every join until the character is answered, and on any
        // join where there are points waiting to be assigned.
        if (!ElysiumCharacter.hasChosen(player)
                || ElysiumCharacter.getUnspentPoints(player) > 0) {
            ElysiumNetwork.sendSheet(player);
        }
    }
}
