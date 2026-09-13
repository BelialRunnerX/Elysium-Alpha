package com.elysium.dungeons.event;

import com.elysium.dungeons.ElysiumDungeons;
import com.elysium.dungeons.level.DungeonInstance;
import com.elysium.dungeons.level.DungeonInstances;
import com.elysium.dungeons.level.DungeonTravel;
import net.minecraft.ChatFormatting;
import net.minecraft.network.chat.Component;
import net.minecraft.server.MinecraftServer;
import net.minecraft.server.level.ServerPlayer;
import net.minecraft.world.entity.LivingEntity;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.neoforge.event.entity.living.LivingDeathEvent;
import net.neoforged.neoforge.event.entity.player.PlayerEvent;
import net.neoforged.neoforge.event.tick.ServerTickEvent;

/**
 * The three other ways a player stops being inside a dungeon, plus boss-kill
 * depth progression for ROGUELIKE.md §1.
 *
 * Walking out through the return rift is the obvious path and is handled by the
 * portal block. These are the ones that are easy to forget, and each one, left
 * unhandled, breaks the same thing: an instance that never empties, so its
 * portal never rerolls, so the mod's central promise quietly stops being kept
 * for that portal forever.
 *
 * <ul>
 *   <li><b>Logging out inside.</b> The player is gone; the instance still
 *       counts them.</li>
 *   <li><b>Dying inside.</b> Vanilla respawn moves them out of the dimension
 *       without any portal being involved.</li>
 *   <li><b>Being moved out by something else</b> — a command, another mod's
 *       teleport. Caught by the dimension-change event rather than by trying to
 *       enumerate the causes.</li>
 * </ul>
 */
@EventBusSubscriber(modid = ElysiumDungeons.MODID)
public final class DungeonEvents {

    private DungeonEvents() {
    }

    /**
     * Boss kill advances portal depth on the next fresh allocation
     * (see {@link DungeonInstances#leave}).
     */
    @SubscribeEvent
    public static void onBossDeath(LivingDeathEvent event) {
        LivingEntity entity = event.getEntity();
        if (entity.level().isClientSide()) {
            return;
        }
        if (!entity.level().dimension().equals(ElysiumDungeons.DUNGEON_LEVEL)) {
            return;
        }
        if (!entity.getPersistentData().getBoolean("ElysiumRiftBoss")) {
            return;
        }
        MinecraftServer server = entity.getServer();
        if (server == null) {
            return;
        }
        DungeonInstances instances = DungeonInstances.get(server);
        DungeonInstance instance = instances.instanceAt(entity.blockPosition());
        if (instance == null || instance.isBossDefeated()) {
            return;
        }
        instance.setBossDefeated(true);
        instances.setDirty();
        if (event.getSource().getEntity() instanceof ServerPlayer player) {
            player.displayClientMessage(
                    Component.translatable(
                                    "elysiumdungeons.message.boss_cleared",
                                    instance.getDepth())
                            .withStyle(ChatFormatting.GOLD),
                    true);
        }
        ElysiumDungeons.LOGGER.info(
                "Rift boss defeated in {} (depth {})", instance, instance.getDepth());
    }

    /**
     * The travel cooldown, ticked once per server tick.
     *
     * On the server tick rather than the player tick because it is one map of a
     * handful of entries, and ticking it per player would tick it once per
     * player per tick for no benefit.
     */
    @SubscribeEvent
    public static void onServerTick(ServerTickEvent.Post event) {
        DungeonTravel.tickCooldowns();
    }

    /** Logged out inside: the instance must stop counting them. */
    @SubscribeEvent
    public static void onLogout(PlayerEvent.PlayerLoggedOutEvent event) {
        if (!(event.getEntity() instanceof ServerPlayer player)) {
            return;
        }
        if (!player.level().dimension().equals(ElysiumDungeons.DUNGEON_LEVEL)) {
            return;
        }
        MinecraftServer server = player.getServer();
        if (server != null) {
            DungeonTravel.recordDeparture(server, player.getUUID(), player.blockPosition());
        }
    }

    /**
     * Died inside, or was moved out by anything that is not a rift.
     *
     * Uses the dimension the player <em>left</em>, not the one they arrived in,
     * because the question is whether they were in a dungeon a moment ago.
     */
    @SubscribeEvent
    public static void onChangeDimension(PlayerEvent.PlayerChangedDimensionEvent event) {
        if (!event.getFrom().equals(ElysiumDungeons.DUNGEON_LEVEL)) {
            return;
        }
        if (event.getTo().equals(ElysiumDungeons.DUNGEON_LEVEL)) {
            return;
        }
        if (!(event.getEntity() instanceof ServerPlayer player)) {
            return;
        }
        MinecraftServer server = player.getServer();
        if (server == null) {
            return;
        }
        // The event's position is where they arrived, which is in the
        // destination dimension and no use for finding the dungeon they left.
        // So the instance is found by asking which one is still counting this
        // player - and only that one, because removing them from instances
        // they were never in would retire other people's dungeons.
        DungeonInstances instances = DungeonInstances.get(server);
        for (DungeonInstance instance : instances.all()) {
            if (instance.contains(player.getUUID())) {
                instances.leave(instance, player.getUUID());
            }
        }
    }
}
