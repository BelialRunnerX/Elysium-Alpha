package com.elysium.event;

import com.elysium.Elysium;
import com.elysium.item.ElysiumRuneEffects;
import com.elysium.item.ElysiumSocketable;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.ItemStack;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.neoforge.event.tick.PlayerTickEvent;

/**
 * Drives the worn-armour rune effects.
 *
 * NeoForge removed {@code IItemExtension#onArmorTick} in 1.21, so the old
 * override on {@code ElysiumArmorItem} would simply never have been called.
 * A player tick handler is the supported replacement.
 */
@EventBusSubscriber(modid = Elysium.MODID)
public final class ElysiumArmorTickHandler {

    private ElysiumArmorTickHandler() {
    }

    /** Effects last 3 seconds, so refreshing once a second is plenty. */
    private static final int INTERVAL = 20;

    @SubscribeEvent
    public static void onPlayerTick(PlayerTickEvent.Post event) {
        Player player = event.getEntity();
        if (player.level().isClientSide()) {
            return;
        }
        if (player.tickCount % INTERVAL != 0) {
            return;
        }

        boolean carryingElysium = false;
        for (ItemStack stack : player.getArmorSlots()) {
            if (stack.getItem() instanceof ElysiumSocketable) {
                carryingElysium = true;
                ElysiumRuneEffects.applyRuneEffects(player, stack);
            }
        }

        // A socketed weapon or tool in hand pulls its own weight.
        ItemStack held = player.getMainHandItem();
        if (held.getItem() instanceof ElysiumSocketable) {
            carryingElysium = true;
            ElysiumRuneEffects.applyRuneEffects(player, held);
        }

        if (!carryingElysium) {
            return;
        }

        // Utility runes act on the whole set, so they resolve once rather than
        // once per piece — otherwise four Stabilizers would heal four times for
        // each of the four pieces.
        ElysiumRuneEffects.applyStabilizer(player);
        ElysiumRuneEffects.applyBarrier(player);
    }
}
