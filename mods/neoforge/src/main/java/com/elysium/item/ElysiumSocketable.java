package com.elysium.item;

import com.elysium.element.ElysiumElement;
import com.elysium.stats.ElysiumGearStats;
import com.elysium.stats.ElysiumStat;
import com.elysium.stats.ElysiumStatBlock;
import net.minecraft.ChatFormatting;
import net.minecraft.network.chat.Component;
import net.minecraft.world.entity.EquipmentSlotGroup;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.component.ItemAttributeModifiers;

import java.util.List;

/**
 * Anything that can take runes: armour, weapons and tools alike.
 *
 * Socketing used to be armour-only, which made rune alignment a half-idea —
 * a Voidglass hammer with a Void affinity had nothing to align *with*. One
 * interface over all three kinds means a rune is a rune wherever it goes, and
 * the only thing that changes is whether it matches.
 */
public interface ElysiumSocketable {

    /** The element this piece resonates with, or {@code NONE} for inert gear. */
    ElysiumElement getElement();

    /** The tier this item was registered at, ignoring ascension. */
    int getTier();

    /**
     * How much of the wielder's Strength this item turns into damage.
     *
     * The stat system's rule is {@code base damage x item multiplier}: the
     * character supplies the base, the item decides how much of it lands. A
     * chestplate multiplies nothing, which is why the default is 1.0 and only
     * things you swing override it.
     */
    default float getDamageMultiplier() {
        return 1.0F;
    }

    /** The tier this particular stack is at, including ascension. */
    default int getEffectiveTier(ItemStack stack) {
        int ascended = ElysiumSockets.gearData(stack).ascendedTier();
        return ascended >= 0 ? ascended : getTier();
    }

    default List<ElysiumRuneItem.RuneType> getSocketedRunes(ItemStack stack) {
        return ElysiumSockets.socketedRunes(stack);
    }

    default int getMaxRuneSlots(ItemStack stack) {
        return ElysiumSockets.maxSlots(getEffectiveTier(stack));
    }

    default boolean socketRune(ItemStack stack, ElysiumRuneItem.RuneType rune) {
        return ElysiumSockets.socket(stack, getEffectiveTier(stack), rune);
    }

    default ItemAttributeModifiers applyRunes(ItemStack stack,
                                              ItemAttributeModifiers modifiers,
                                              EquipmentSlotGroup group) {
        return ElysiumSockets.applyRunes(stack, getElement(), modifiers, group);
    }

    /**
     * The shared block of tooltip lines: element and tier, Imperial clearance,
     * the stats the piece grants and the level it asks for, then each socketed
     * rune with its alignment called out.
     */
    default void appendSocketTooltip(ItemStack stack, List<Component> tooltip) {
        appendIdentityTooltip(stack, tooltip);
        appendStatTooltip(stack, tooltip);
        appendRuneTooltip(stack, tooltip);
    }

    /**
     * What the piece gives, and what it costs to use.
     *
     * The level requirement is printed whether or not the reader meets it. A
     * tooltip cannot see the player holding it without reaching into
     * client-only code, and guessing wrong about who is looking is worse than
     * simply stating the requirement — a player who reads "Requires level 25"
     * and finds nothing happening has been told exactly why.
     */
    default void appendStatTooltip(ItemStack stack, List<Component> tooltip) {
        int required = ElysiumGearStats.requiredLevel(stack);
        if (required > 0) {
            tooltip.add(Component.translatable("elysium.tooltip.requires_level", required)
                    .withStyle(ChatFormatting.YELLOW));
        }

        ElysiumStatBlock granted = ElysiumGearStats.of(stack);
        for (ElysiumStat stat : ElysiumStat.values()) {
            int amount = granted.get(stat);
            if (amount == 0) {
                continue;
            }
            tooltip.add(Component.literal(" +" + amount + " ")
                    .withStyle(ChatFormatting.DARK_GRAY)
                    .append(stat.getDisplayName()));
        }
    }

    /** Element, tier and Imperial clearance. */
    default void appendIdentityTooltip(ItemStack stack, List<Component> tooltip) {
        int tier = getEffectiveTier(stack);
        tooltip.add(getElement().getDisplayName()
                .copy()
                .append(Component.literal(" · ").withStyle(ChatFormatting.DARK_GRAY))
                .append(ElysiumRarities.getTierComponent(tier)));
        tooltip.add(ElysiumRarities.getClearance(tier));
    }

    /**
     * Socket count and one line per rune, aligned runes flagged in gold. Split
     * from the identity block so armour can slot its counter-matrix line in
     * between without duplicating any of this.
     */
    default void appendRuneTooltip(ItemStack stack, List<Component> tooltip) {
        List<ElysiumRuneItem.RuneType> runes = getSocketedRunes(stack);
        tooltip.add(Component.translatable("elysium.tooltip.runes",
                runes.size(), getMaxRuneSlots(stack)).withStyle(ChatFormatting.DARK_AQUA));

        for (ElysiumRuneItem.RuneType rune : runes) {
            boolean aligned = ElysiumSockets.isAligned(rune, getElement());
            Component line = Component.literal(" • ")
                    .withStyle(ChatFormatting.DARK_GRAY)
                    .append(Component.translatable("elysium.rune." + rune.getId() + ".effect")
                            .withStyle(aligned ? ChatFormatting.AQUA : ChatFormatting.GRAY));
            if (aligned) {
                line = line.copy().append(Component.literal(" ")
                        .append(Component.translatable("elysium.tooltip.aligned")
                                .withStyle(ChatFormatting.GOLD)));
            }
            tooltip.add(line);
        }
    }
}
