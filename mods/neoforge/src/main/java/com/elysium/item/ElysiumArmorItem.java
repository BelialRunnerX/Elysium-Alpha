package com.elysium.item;

import net.minecraft.world.entity.ai.attributes.AttributeModifier;
import net.minecraft.world.entity.ai.attributes.Attributes;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.ArmorItem;
import net.minecraft.world.item.ArmorMaterial;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.item.Rarity;
import net.minecraft.world.level.Level;

import java.util.List;
import java.util.UUID;

public class ElysiumArmorItem extends ArmorItem {
    
    private final int element;
    private final int tier;

    public ElysiumArmorItem(ArmorMaterial material, ArmorItem.Type type, Properties properties, int element, int tier) {
        super(material, type, properties);
        this.element = element;
        this.tier = tier;
    }

    public int getElement() {
        return element;
    }

    public int getTier() {
        return tier;
    }

    @Override
    public Rarity getRarity(ItemStack stack) {
        return ElysiumRarities.getRarityFromTier(tier);
    }

    @Override
    public boolean isFoil(ItemStack stack) {
        return this.tier >= 3;
    }

    public String getElementName() {
        return switch (element) {
            case 0 -> "Void";
            case 1 -> "Plasma";
            case 2 -> "Neural";
            case 3 -> "Dimensional";
            case 4 -> "Kinetic";
            default -> "Unknown";
        };
    }

    // === Rune Socket System ===

    public List<ElysiumRuneItem.RuneType> getSocketedRunes(ItemStack stack) {
        List<ElysiumRuneItem.RuneType> runes = new java.util.ArrayList<>();
        net.minecraft.nbt.CompoundTag tag = stack.getTag();
        if (tag != null && tag.contains("ElysiumRunes", net.minecraft.nbt.Tag.TAG_LIST)) {
            net.minecraft.nbt.ListTag list = tag.getList("ElysiumRunes", net.minecraft.nbt.Tag.TAG_STRING);
            for (int i = 0; i < list.size(); i++) {
                try {
                    runes.add(ElysiumRuneItem.RuneType.valueOf(list.getString(i)));
                } catch (IllegalArgumentException ignored) {}
            }
        }
        return runes;
    }

    public boolean socketRune(ItemStack armor, ElysiumRuneItem.RuneType rune) {
        List<ElysiumRuneItem.RuneType> current = getSocketedRunes(armor);
        int maxSlots = getMaxRuneSlots();
        if (current.size() >= maxSlots) return false;
        if (current.contains(rune)) return false;

        net.minecraft.nbt.CompoundTag tag = armor.getOrCreateTag();
        net.minecraft.nbt.ListTag list = tag.contains("ElysiumRunes", net.minecraft.nbt.Tag.TAG_LIST) ? 
            tag.getList("ElysiumRunes", net.minecraft.nbt.Tag.TAG_STRING) : new net.minecraft.nbt.ListTag();
        list.add(net.minecraft.nbt.StringTag.valueOf(rune.name()));
        tag.put("ElysiumRunes", list);
        return true;
    }

    public int getMaxRuneSlots() {
        return switch (this.tier) {
            case 0, 1 -> 1;
            case 2, 3 -> 2;
            case 4, 5 -> 3;
            default -> 1;
        };
    }

    public boolean canAscend() {
        return this.tier < 4;
    }

    public int getNextTier() {
        return Math.min(this.tier + 1, 5);
    }

    public boolean rerollPsionicAffix(ItemStack armor, ElysiumRuneItem.RuneType rune) {
        // Placeholder for Apotheosis affix reroll
        return true;
    }

    @Override
    public void onArmorTick(ItemStack stack, Level level, Player player) {
        if (!level.isClientSide) {
            // Apply rune effects
            ElysiumRuneEffects.applyRuneEffects(player, stack);
        }
    }

    /**
     * Applies reforged stats as attribute modifiers.
     * This is called when the armor is equipped.
     */
    public void applyReforgedStats(ItemStack stack, Player player) {
        net.minecraft.nbt.CompoundTag tag = stack.getTag();
        if (tag != null && tag.getBoolean("ElysiumReforged")) {
            int armorBonus = tag.getInt("ElysiumReforgedArmor");
            int healthBonus = tag.getInt("ElysiumReforgedHealth");
            int speedBonus = tag.getInt("ElysiumReforgedSpeed");
            
            // Apply attribute modifiers
            if (armorBonus > 0) {
                player.getAttribute(Attributes.ARMOR).addTransientModifier(
                    new AttributeModifier(UUID.randomUUID(), "elysium_reforged_armor", armorBonus, AttributeModifier.Operation.ADDITION)
                );
            }
            if (healthBonus > 0) {
                player.getAttribute(Attributes.MAX_HEALTH).addTransientModifier(
                    new AttributeModifier(UUID.randomUUID(), "elysium_reforged_health", healthBonus, AttributeModifier.Operation.ADDITION)
                );
            }
            if (speedBonus > 0) {
                player.getAttribute(Attributes.MOVEMENT_SPEED).addTransientModifier(
                    new AttributeModifier(UUID.randomUUID(), "elysium_reforged_speed", speedBonus * 0.01, AttributeModifier.Operation.MULTIPLY_BASE)
                );
            }
        }
    }
}