package com.elysium.event;

import com.elysium.Elysium;
import com.elysium.character.ElysiumCharacter;
import com.elysium.entity.ElysiumFaction;
import com.elysium.entity.ImperialEnforcer;
import com.elysium.entity.UnswornRaider;
import com.elysium.standing.ElysiumStanding;
import net.minecraft.core.BlockPos;
import net.minecraft.server.level.ServerLevel;
import net.minecraft.util.Mth;
import net.minecraft.util.RandomSource;
import net.minecraft.world.entity.Entity;
import net.minecraft.world.entity.Mob;
import net.minecraft.world.entity.MobSpawnType;
import net.minecraft.world.entity.item.ItemEntity;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.Item;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.level.block.Block;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.neoforge.event.entity.living.LivingDeathEvent;
import net.neoforged.neoforge.event.level.BlockEvent;
import net.neoforged.neoforge.event.tick.PlayerTickEvent;
import org.jetbrains.annotations.Nullable;

import java.util.List;

/**
 * The two loops, and the clock that runs them down.
 *
 * <pre>
 *   kill Unsworn  →  Favor      →  more Unsworn spawn
 *   kill Empire   →  Suspicion  →  more Empire spawn
 * </pre>
 *
 * Both sides are dispatched the same way and on the same schedule, so neither
 * loop is mechanically privileged — the difference is only in what each one
 * pays out, which {@code ElysiumLootHandler} decides.
 *
 * There is deliberately no spawn suppression here. An earlier cut had Favor
 * calling off natural spawns, which reads well as "the Empire keeps the roads
 * clear" but pulls against the loop: a meter that both summons enemies and
 * removes them nets out to nothing.
 */
@EventBusSubscriber(modid = Elysium.MODID)
public final class ElysiumStandingHandler {

    private ElysiumStandingHandler() {
    }

    /** How often the world considers sending someone from either side. */
    private static final int DISPATCH_INTERVAL = 200;

    /**
     * The chance an ordinary hostile counts toward Favor at all.
     *
     * This is the only rung on the ladder available to a player who has not
     * touched Elysium content yet — no workstation, no raider to kill, no
     * Empire mob to anger. At one point in three it takes about seventy-five
     * kills to reach Recognised, which is a long cave trip rather than a mob
     * farm. Any higher and a single night tops the meter out on its own.
     */
    private static final float INCIDENTAL_FAVOR_CHANCE = 0.34F;

    private static final double DISPATCH_MIN_RANGE = 20.0D;
    private static final double DISPATCH_MAX_RANGE = 36.0D;
    private static final double CROWD_RADIUS = 64.0D;

    // ------------------------------------------------------------------
    // Decay and dispatch
    // ------------------------------------------------------------------

    @SubscribeEvent
    public static void onPlayerTick(PlayerTickEvent.Post event) {
        Player player = event.getEntity();
        if (!(player.level() instanceof ServerLevel level)) {
            return;
        }

        // Both meters bleed off at the same rate. Standing is something you
        // hold by continuing to act, not something you bank once — but only
        // above the notice threshold, or the first climb runs uphill against
        // the clock and nothing is reachable. See ElysiumStanding.decays.
        if (player.tickCount % ElysiumStanding.DECAY_INTERVAL == 0) {
            if (ElysiumStanding.decays(ElysiumStanding.getFavor(player))) {
                ElysiumStanding.addFavor(player, -1);
            }
            if (ElysiumStanding.decays(ElysiumStanding.getSuspicion(player))) {
                ElysiumStanding.addSuspicion(player, -ElysiumPassiveHandler.decayRate(player));
            }
        }

        if (player.tickCount % DISPATCH_INTERVAL == 0) {
            considerEmpireDispatch(level, player);
            considerUnswornDispatch(level, player);
        }
    }

    private static void considerEmpireDispatch(ServerLevel level, Player player) {
        int suspicion = ElysiumStanding.getSuspicion(player);
        if (!shouldDispatch(level, player, ElysiumStanding.empireSpawnChance(suspicion),
                ElysiumStanding.spawnCap(suspicion), ImperialEnforcer.class)) {
            return;
        }

        BlockPos pos = findSpawnPos(level, player);
        if (pos == null) {
            return;
        }
        ImperialEnforcer enforcer = Elysium.IMPERIAL_ENFORCER.get().create(level);
        if (enforcer == null) {
            return;
        }
        place(level, player, enforcer, pos);
        enforcer.equipForBand(ElysiumStanding.bandOf(suspicion));
    }

    private static void considerUnswornDispatch(ServerLevel level, Player player) {
        int favor = ElysiumStanding.getFavor(player);
        if (!shouldDispatch(level, player, ElysiumStanding.unswornSpawnChance(favor),
                ElysiumStanding.spawnCap(favor), UnswornRaider.class)) {
            return;
        }

        BlockPos pos = findSpawnPos(level, player);
        if (pos == null) {
            return;
        }
        UnswornRaider raider = Elysium.UNSWORN_RAIDER.get().create(level);
        if (raider == null) {
            return;
        }
        place(level, player, raider, pos);
        raider.equipForBand(ElysiumStanding.bandOf(favor), level.getRandom());
    }

    private static boolean shouldDispatch(ServerLevel level, Player player, float chance,
                                          int cap, Class<? extends Mob> type) {
        if (chance <= 0.0F || cap <= 0) {
            return false;
        }
        if (level.getRandom().nextFloat() >= chance) {
            return false;
        }
        List<? extends Mob> nearby = level.getEntitiesOfClass(type,
                player.getBoundingBox().inflate(CROWD_RADIUS));
        return nearby.size() < cap;
    }

    /**
     * finalizeSpawn runs the vanilla kit-out for the base mob, so any Elysium
     * gear has to go on after this or it would be overwritten.
     */
    private static void place(ServerLevel level, Player player, Mob mob, BlockPos pos) {
        mob.moveTo(pos.getX() + 0.5D, pos.getY(), pos.getZ() + 0.5D,
                level.getRandom().nextFloat() * 360.0F, 0.0F);
        mob.finalizeSpawn(level, level.getCurrentDifficultyAt(pos), MobSpawnType.EVENT, null);
        mob.setTarget(player);
        level.addFreshEntity(mob);
    }

    /**
     * Somewhere out of sight but within earshot: far enough that they are not
     * simply dropped on the player's head, close enough that they arrive.
     */
    @Nullable
    private static BlockPos findSpawnPos(ServerLevel level, Player player) {
        RandomSource random = level.getRandom();
        for (int attempt = 0; attempt < 12; attempt++) {
            double angle = random.nextDouble() * Math.PI * 2.0D;
            double distance = DISPATCH_MIN_RANGE
                    + random.nextDouble() * (DISPATCH_MAX_RANGE - DISPATCH_MIN_RANGE);
            int x = Mth.floor(player.getX() + Math.cos(angle) * distance);
            int z = Mth.floor(player.getZ() + Math.sin(angle) * distance);
            int startY = Mth.floor(player.getY()) + 6;

            for (int y = startY; y > startY - 16; y--) {
                BlockPos pos = new BlockPos(x, y, z);
                if (level.isEmptyBlock(pos)
                        && level.isEmptyBlock(pos.above())
                        && !level.isEmptyBlock(pos.below())) {
                    return pos;
                }
            }
        }
        return null;
    }

    /** The Reclaimer's second ingot, matched to the vein it came out of. */
    private static void dropBonusIngot(Player player, BlockPos pos, Block block) {
        Item ingot;
        if (block == Elysium.NEUTRONIUM_ORE.get()) {
            ingot = Elysium.NEUTRONIUM_INGOT.get();
        } else if (block == Elysium.VOIDGLASS_ORE.get()) {
            ingot = Elysium.VOIDGLASS_INGOT.get();
        } else {
            ingot = Elysium.AETHERIUM_INGOT.get();
        }
        player.level().addFreshEntity(new ItemEntity(player.level(),
                pos.getX() + 0.5D, pos.getY() + 0.5D, pos.getZ() + 0.5D,
                new ItemStack(ingot)));
    }

    // ------------------------------------------------------------------
    // Feeding the meters
    // ------------------------------------------------------------------

    @SubscribeEvent
    public static void onDeath(LivingDeathEvent event) {
        if (event.getEntity().level().isClientSide()) {
            return;
        }
        Entity killer = event.getSource().getEntity();
        if (!(killer instanceof Player player)) {
            return;
        }

        Entity victim = event.getEntity();
        boolean named = ElysiumFaction.isNamedCombatant(victim);

        // Character experience. A named faction combatant is worth several
        // ordinary hostiles, because the loop that spawns them is the one the
        // level track is meant to follow.
        ElysiumCharacter.addXp(player, named ? 14 : 4);

        switch (ElysiumFaction.of(victim)) {
            case EMPIRE ->
                // Killing the Empire's own is the one thing it cannot overlook.
                    ElysiumStanding.addSuspicion(player, 12);
            case UNSWORN -> {
                // A named raider is a sanctioned target and always counts. An
                // ordinary hostile counts sometimes, or a night of mob farming
                // would top the meter out on its own.
                if (named) {
                    ElysiumStanding.addFavor(player, 4);
                } else if (player.getRandom().nextFloat() < INCIDENTAL_FAVOR_CHANCE) {
                    ElysiumStanding.addFavor(player, 1);
                }
            }
            default -> {
                // Neutral. The Empire does not care.
            }
        }
    }

    @SubscribeEvent
    public static void onBlockBreak(BlockEvent.BreakEvent event) {
        Player player = event.getPlayer();
        if (player == null || player.level().isClientSide()) {
            return;
        }

        Block block = event.getState().getBlock();

        // Ore a player put there is not ore they found. Without this, Silk
        // Touch plus a single vein was an unbounded experience loop — see
        // ElysiumPlacedOre.
        if (ElysiumPlacedOre.isElysiumOre(block) && ElysiumPlacedOre.wasPlaced(event.getPos())) {
            return;
        }

        int suspicion = 0;
        if (block == Elysium.NEUTRONIUM_ORE.get()) {
            suspicion = 4;
        } else if (block == Elysium.VOIDGLASS_ORE.get() || block == Elysium.AETHERIUM_ORE.get()) {
            suspicion = 2;
        }

        if (suspicion > 0) {
            ElysiumStanding.addSuspicion(player, suspicion);
            ElysiumCharacter.addXp(player, 6);

            // Prospector: a Reclaimer sometimes gets a second ingot out of a
            // vein. Dropped at the block rather than added to the loot table,
            // because the table has no idea who is holding the pickaxe.
            if (ElysiumPassiveHandler.doublesOre(player)) {
                dropBonusIngot(player, event.getPos(), block);
            }
        }
    }
}
