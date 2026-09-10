package com.elysium.affix;

import shadows.apotheosis.adventure.affix.Affix;
import shadows.apotheosis.adventure.affix.AffixType;
import net.minecraft.world.item.ItemStack;

public class ElysiumAffix extends Affix {

    private final String name;
    private final float value;

    public ElysiumAffix(String name, float value) {
        super(AffixType.ARMOR);
        this.name = name;
        this.value = value;
    }

    @Override
    public float getMin() {
        return value * 0.5f;
    }

    @Override
    public float getMax() {
        return value * 1.5f;
    }

    @Override
    public boolean canApplyTo(ItemStack stack) {
        // Only apply to Elysium armor
        return stack.getItem() instanceof com.elysium.item.ElysiumArmorItem;
    }
}