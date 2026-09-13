package com.elysium.character;

import com.elysium.Elysium;
import com.elysium.stats.ElysiumStat;
import com.elysium.stats.ElysiumStatBlock;
import com.mojang.serialization.Codec;
import net.minecraft.ChatFormatting;
import net.minecraft.network.chat.Component;
import net.minecraft.world.entity.player.Player;
import net.neoforged.neoforge.attachment.AttachmentType;
import net.neoforged.neoforge.registries.DeferredRegister;
import net.neoforged.neoforge.registries.NeoForgeRegistries;

import java.util.Map;
import java.util.function.Supplier;

/**
 * Who a player is: race, class, level, and the points they have spent.
 *
 * All of it lives in data attachments alongside Favor and Suspicion, and all
 * of it survives death. A character sheet you lose on a bad fall is not a
 * character sheet.
 *
 * <h2>Levelling, and why it has no ceiling</h2>
 *
 * Experience comes from doing Elysium things — killing the factions' mobs,
 * cutting their ore, working at their benches — and the requirement per level
 * grows linearly, so total experience grows as the square of level. There is
 * no maximum. That is not an oversight: armour requires a level to wear and
 * ascension raises armour tiers forever, so the level track has to be able to
 * follow it forever too.
 *
 * Vanilla experience is left alone entirely. Spending green levels on an anvil
 * should never take your chestplate off.
 */
public final class ElysiumCharacter {

    private ElysiumCharacter() {
    }

    public static final DeferredRegister<AttachmentType<?>> ATTACHMENTS =
            DeferredRegister.create(NeoForgeRegistries.ATTACHMENT_TYPES, Elysium.MODID);

    /** Free points granted per level, on top of race and class growth. */
    public static final int POINTS_PER_LEVEL = 2;

    // ------------------------------------------------------------------
    // Attachments
    // ------------------------------------------------------------------

    /** Empty until chosen. An empty race is what triggers the first-join picker. */
    public static final Supplier<AttachmentType<String>> RACE =
            ATTACHMENTS.register("race", () -> AttachmentType.<String>builder(() -> "")
                    .serialize(Codec.STRING)
                    .copyOnDeath()
                    .build());

    public static final Supplier<AttachmentType<String>> CLAZZ =
            ATTACHMENTS.register("class", () -> AttachmentType.<String>builder(() -> "")
                    .serialize(Codec.STRING)
                    .copyOnDeath()
                    .build());

    public static final Supplier<AttachmentType<Integer>> LEVEL =
            ATTACHMENTS.register("level", () -> AttachmentType.<Integer>builder(() -> 1)
                    .serialize(Codec.INT)
                    .copyOnDeath()
                    .build());

    public static final Supplier<AttachmentType<Integer>> XP =
            ATTACHMENTS.register("xp", () -> AttachmentType.<Integer>builder(() -> 0)
                    .serialize(Codec.INT)
                    .copyOnDeath()
                    .build());

    public static final Supplier<AttachmentType<Integer>> UNSPENT =
            ATTACHMENTS.register("unspent", () -> AttachmentType.<Integer>builder(() -> 0)
                    .serialize(Codec.INT)
                    .copyOnDeath()
                    .build());

    /**
     * Whether this player has ever been handed a Codex.
     *
     * Kept separately from the race because the picker can be escaped by
     * quitting: keying the grant on "has no race yet" handed out a fresh Codex
     * on every relog to anyone who had not answered it.
     */
    public static final Supplier<AttachmentType<Boolean>> CODEX_GIVEN =
            ATTACHMENTS.register("codex_given", () -> AttachmentType.<Boolean>builder(() -> false)
                    .serialize(Codec.BOOL)
                    .copyOnDeath()
                    .build());

    /**
     * Points the player has assigned by hand. Stored as a plain string map so
     * a save written before a stat existed still loads.
     */
    public static final Supplier<AttachmentType<Map<String, Integer>>> SPENT =
            ATTACHMENTS.register("spent", () -> AttachmentType.<Map<String, Integer>>builder(Map::of)
                    .serialize(ElysiumStatBlock.MAP_CODEC)
                    .copyOnDeath()
                    .build());

    // ------------------------------------------------------------------
    // Identity
    // ------------------------------------------------------------------

    /** @return the chosen race, or null when the player has not chosen yet */
    public static ElysiumRace getRace(Player player) {
        return ElysiumRace.byId(player.getData(RACE.get()));
    }

    /** @return the chosen class, or null when the player has not chosen yet */
    public static ElysiumClass getElysiumClass(Player player) {
        return ElysiumClass.byId(player.getData(CLAZZ.get()));
    }

    public static boolean hasChosen(Player player) {
        return getRace(player) != null && getElysiumClass(player) != null;
    }

    public static void setRace(Player player, ElysiumRace race) {
        player.setData(RACE.get(), race.getId());
    }

    public static void setElysiumClass(Player player, ElysiumClass value) {
        player.setData(CLAZZ.get(), value.getId());
    }

    public static boolean hasCodex(Player player) {
        return player.getData(CODEX_GIVEN.get());
    }

    public static void markCodexGiven(Player player) {
        player.setData(CODEX_GIVEN.get(), true);
    }

    // ------------------------------------------------------------------
    // Level and experience
    // ------------------------------------------------------------------

    public static int getLevel(Player player) {
        return Math.max(1, player.getData(LEVEL.get()));
    }

    public static int getXp(Player player) {
        return player.getData(XP.get());
    }

    public static int getUnspentPoints(Player player) {
        return player.getData(UNSPENT.get());
    }

    /**
     * Experience needed to leave the given level.
     *
     * Linear in level, so the cumulative cost is quadratic — the classic curve
     * that keeps early levels quick and late ones meaningful without ever
     * becoming impossible.
     */
    public static int xpToNext(int level) {
        return 60 + 40 * level;
    }

    /**
     * Awards experience and levels up as many times as it earns.
     *
     * @return how many levels were gained, so the caller can announce it
     */
    public static int addXp(Player player, int amount) {
        if (amount <= 0) {
            return 0;
        }
        int xp = getXp(player) + amount;
        int level = getLevel(player);
        int gained = 0;

        while (xp >= xpToNext(level)) {
            xp -= xpToNext(level);
            level++;
            gained++;
            // A runaway award should not lock the server in this loop.
            if (gained > 1000) {
                break;
            }
        }

        player.setData(XP.get(), xp);
        if (gained > 0) {
            player.setData(LEVEL.get(), level);
            player.setData(UNSPENT.get(), getUnspentPoints(player) + gained * POINTS_PER_LEVEL);
            player.displayClientMessage(Component.translatable(
                    "elysium.level.up", level, gained * POINTS_PER_LEVEL)
                    .withStyle(ChatFormatting.GOLD), false);
        }
        return gained;
    }

    // ------------------------------------------------------------------
    // Spent points
    // ------------------------------------------------------------------

    public static ElysiumStatBlock getSpent(Player player) {
        return ElysiumStatBlock.fromMap(player.getData(SPENT.get()));
    }

    /**
     * Spends one free point on a stat.
     *
     * @return false when there is nothing left to spend
     */
    public static boolean spendPoint(Player player, ElysiumStat stat) {
        int unspent = getUnspentPoints(player);
        if (unspent <= 0) {
            return false;
        }
        player.setData(UNSPENT.get(), unspent - 1);
        player.setData(SPENT.get(), getSpent(player).with(stat, 1).toMap());
        return true;
    }

    /**
     * Hands every spent point back.
     *
     * Offered because a stat sheet a player cannot correct is a stat sheet
     * they will reroll a character to escape.
     */
    public static void respec(Player player) {
        ElysiumStatBlock spent = getSpent(player);
        int returned = 0;
        for (ElysiumStat stat : ElysiumStat.values()) {
            returned += spent.get(stat);
        }
        player.setData(SPENT.get(), Map.of());
        player.setData(UNSPENT.get(), getUnspentPoints(player) + returned);
    }
}
