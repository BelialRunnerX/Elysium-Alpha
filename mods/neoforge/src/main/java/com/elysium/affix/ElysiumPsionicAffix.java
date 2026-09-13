package com.elysium.affix;

import shadows.apotheosis.adventure.affix.Affix;
import shadows.apotheosis.adventure.affix.AffixType;
import net.minecraft.world.item.ItemStack;
import com.elysium.item.ElysiumArmorItem;
import net.minecraft.world.entity.ai.attributes.AttributeModifier;
import net.minecraft.world.entity.ai.attributes.Attributes;
import java.util.UUID;
import java.util.function.BiConsumer;

public class ElysiumPsionicAffix extends Affix {

    private final String element;
    private final float minValue;
    private final float maxValue;

    public ElysiumPsionicAffix(String element, float minValue, float maxValue) {
        super(AffixType.ARMOR);
        this.element = element;
        this.minValue = minValue;
        this.maxValue = maxValue;
    }

    @Override
    public float getMin() {
        return minValue;
    }

    @Override
    public float getMax() {
        return maxValue;
    }

    @Override
    public boolean canApplyTo(ItemStack stack) {
        return stack.getItem() instanceof ElysiumArmorItem;
    }

    public String getElement() {
        return element;
    }

    @Override
    public void addModifiers(ItemStack stack, float level, BiConsumer<net.minecraft.world.entity.ai.attributes.Attribute, AttributeModifier> consumer) {
        double amount = minValue + (maxValue - minValue) * level;
        
        AttributeModifier modifier = switch (element) {
            case "Void" -> new AttributeModifier(
                UUID.randomUUID(), 
                "elysium_void_resistance", 
                amount, 
                AttributeModifier.Operation.ADDITION
            );
            case "Plasma" -> new AttributeModifier(
                UUID.randomUUID(), 
                "elysium_plasma_damage", 
                amount, 
                AttributeModifier.Operation.MULTIPLY_BASE
            );
            case "Neural" -> new AttributeModifier(
                UUID.randomUUID(), 
                "elysium_neural_haste", 
                amount, 
                AttributeModifier.Operation.MULTIPLY_BASE
            );
            case "Dimensional" -> new AttributeModifier(
                UUID.randomUUID(), 
                "elysium_dimensional_mobility", 
                amount, 
                AttributeModifier.Operation.ADDITION
            );
            case "Kinetic" -> new AttributeModifier(
                UUID.randomUUID(), 
                "elysium_kinetic_force", 
                amount, 
                AttributeModifier.Operation.MULTIPLY_BASE
            );
            default -> new AttributeModifier(
                UUID.randomUUID(), 
                "elysium_generic", 
                amount, 
                AttributeModifier.Operation.ADDITION
            );
        };
        
        // Apply the modifier to the appropriate attribute
        switch (element) {
            case "Void" -> consumer.accept(Attributes.KNOCKBACK_RESISTANCE, modifier);
            case "Plasma", "Kinetic" -> consumer.accept(Attributes.ATTACK_DAMAGE, modifier);
            case "Neural" -> consumer.accept(Attributes.ATTACK_SPEED, modifier);
            case "Dimensional" -> consumer.accept(Attributes.MOVEMENT_SPEED, modifier);
            default -> consumer.accept(Attributes.ARMOR, modifier);
        }
    }
}