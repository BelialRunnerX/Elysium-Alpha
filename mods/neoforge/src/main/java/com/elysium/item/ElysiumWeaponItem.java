package com.elysium.item;

import com.elysium.element.ElysiumElement;
import net.minecraft.ChatFormatting;
import net.minecraft.network.chat.Component;
import net.minecraft.world.entity.EquipmentSlotGroup;
import net.minecraft.world.item.Item;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.SwordItem;
import net.minecraft.world.item.Tier;
import net.minecraft.world.item.TooltipFlag;
import net.minecraft.world.item.component.ItemAttributeModifiers;

import java.util.List;

/**
 * An Elysium weapon.
 *
 * Weapons carry an element and a tier, and that pairing is the whole point:
 * {@code ElysiumCombatHandler} reads the element to decide whether the swing
 * has the advantage, and the tier to decide how large it is.
 *
 * Extends {@link SwordItem} so vanilla's sweep, enchantability and
 * block-breaking behaviour come for free. Attack damage and speed are declared
 * through {@code SwordItem.createAttributes}, which is where 1.21.1 expects
 * them — an item's attributes are a data component now, not an override.
 */
public class ElysiumWeaponItem extends SwordItem implements ElysiumSocketable {

    private final ElysiumElement element;
    private final int tier;

    public ElysiumWeaponItem(Tier material,
                             ElysiumElement element,
                             int tier,
                             float attackDamage,
                             float attackSpeed) {
        super(material, new Item.Properties()
                .rarity(ElysiumRarities.getRarityFromTier(tier))
                .attributes(SwordItem.createAttributes(material, attackDamage, attackSpeed)));
        this.element = element;
        this.tier = tier;
    }

    @Override
    public ElysiumElement getElement() {
        return element;
    }

    @Override
    public int getTier() {
        return tier;
    }

    /**
     * Socketed runes ride on the weapon the same way they ride on armour, and
     * a rune matching the blade's element bites harder. Attributes are declared
     * per-stack because the sockets are per-stack.
     */
    @Override
    public ItemAttributeModifiers getDefaultAttributeModifiers(ItemStack stack) {
        return applyRunes(stack, super.getDefaultAttributeModifiers(stack),
                EquipmentSlotGroup.MAINHAND);
    }

    /**
     * A blade is what Strength is for. Everything in the Elysium weapon line
     * turns the wielder's base damage into more than an empty hand would.
     */
    @Override
    public float getDamageMultiplier() {
        return 1.6F;
    }

    /** How much extra damage a favourable matchup deals. */
    public float getAdvantage() {
        return ElysiumRarities.getAdvantage(tier);
    }

    @Override
    public boolean isFoil(ItemStack stack) {
        return tier >= ElysiumRarities.LEGENDARY;
    }

    @Override
    public void appendHoverText(ItemStack stack,
                                Item.TooltipContext context,
                                List<Component> tooltip,
                                TooltipFlag flag) {
        super.appendHoverText(stack, context, tooltip, flag);

        appendIdentityTooltip(stack, tooltip);

        ElysiumElement[] countered = element.counters();
        if (countered.length == 2) {
            int percent = Math.round(getAdvantage() * 100.0F);
            tooltip.add(Component.translatable("elysium.tooltip.advantage",
                            percent,
                            countered[0].getDisplayName(),
                            countered[1].getDisplayName())
                    .withStyle(ChatFormatting.GRAY));
        }

        appendStatTooltip(stack, tooltip);
        appendRuneTooltip(stack, tooltip);
    }
}
