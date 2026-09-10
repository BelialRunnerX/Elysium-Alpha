package com.elysium.item;

import com.elysium.affix.ElysiumAffix;
import com.elysium.affix.ElysiumAffixes;
import com.elysium.element.ElysiumElement;
import net.minecraft.world.entity.EquipmentSlotGroup;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.component.ItemAttributeModifiers;

import java.util.ArrayList;
import java.util.List;

/**
 * The rune socket system, shared by armour, weapons and tools.
 *
 * The rule that matters: <b>a rune aligned to the item's element hits
 * harder.</b> A Voidward rune in Voidglass gear is resonating with the metal;
 * the same rune in Neutronium still works, it is just fighting the material.
 * Nothing is ever locked out — mismatched runes give exactly what they always
 * gave, and aligned ones give {@link #ALIGNED_MULTIPLIER} times that. Alignment
 * is a reward, never a gate.
 *
 * Utility runes have no element, so they are never aligned and never penalised.
 * They are the flat option you take when you cannot get a match.
 */
public final class ElysiumSockets {

    private ElysiumSockets() {
    }

    /** What an aligned rune is worth, relative to the same rune mismatched. */
    public static final float ALIGNED_MULTIPLIER = 1.75F;

    // ------------------------------------------------------------------
    // Stack data
    // ------------------------------------------------------------------

    public static ElysiumGearData gearData(ItemStack stack) {
        return stack.getOrDefault(ElysiumComponents.GEAR_DATA.get(), ElysiumGearData.EMPTY);
    }

    public static void setGearData(ItemStack stack, ElysiumGearData data) {
        stack.set(ElysiumComponents.GEAR_DATA.get(), data);
    }

    public static List<ElysiumRuneItem.RuneType> socketedRunes(ItemStack stack) {
        List<ElysiumRuneItem.RuneType> runes = new ArrayList<>();
        for (String name : gearData(stack).runes()) {
            ElysiumRuneItem.RuneType parsed = ElysiumRuneItem.RuneType.byName(name);
            if (parsed != null) {
                runes.add(parsed);
            }
        }
        return runes;
    }

    /**
     * Socket capacity by tier, the same curve for every kind of gear.
     *
     * One slot per two tiers, with no ceiling — the old table stopped at three
     * slots, which quietly capped the whole rune system at the moment
     * ascension stopped being capped.
     */
    public static int maxSlots(int tier) {
        return 1 + Math.max(0, tier) / 2;
    }

    public static boolean socket(ItemStack stack, int tier, ElysiumRuneItem.RuneType rune) {
        List<ElysiumRuneItem.RuneType> current = socketedRunes(stack);
        if (current.size() >= maxSlots(tier) || current.contains(rune)) {
            return false;
        }
        setGearData(stack, gearData(stack).withRuneAdded(rune.name()));
        return true;
    }

    // ------------------------------------------------------------------
    // Alignment
    // ------------------------------------------------------------------

    public static boolean isAligned(ElysiumRuneItem.RuneType rune, ElysiumElement element) {
        ElysiumElement runeElement = rune.getElement();
        return runeElement.isElemental() && runeElement == element;
    }

    public static float alignmentScale(ElysiumRuneItem.RuneType rune, ElysiumElement element) {
        return isAligned(rune, element) ? ALIGNED_MULTIPLIER : 1.0F;
    }

    // ------------------------------------------------------------------
    // Attribute application
    // ------------------------------------------------------------------

    /**
     * Adds one modifier per socketed rune, scaled by whether it aligns.
     *
     * The slot group is folded into each modifier's id. Without that, a
     * Voidward rune in a helmet and another in a chestplate would produce two
     * modifiers with identical ids on the same attribute, and only one of them
     * would survive.
     */
    public static ItemAttributeModifiers applyRunes(ItemStack stack,
                                                    ElysiumElement element,
                                                    ItemAttributeModifiers modifiers,
                                                    EquipmentSlotGroup group) {
        for (ElysiumRuneItem.RuneType rune : socketedRunes(stack)) {
            ElysiumAffix affix = ElysiumAffixes.forRune(rune);
            if (affix == null) {
                // Utility runes are behaviour, not attributes. Handled by the
                // combat and tick handlers instead.
                continue;
            }
            modifiers = modifiers.withModifierAdded(affix.getAttribute(),
                    affix.createModifier(1.0F, alignmentScale(rune, element), group), group);
        }
        return modifiers;
    }
}
