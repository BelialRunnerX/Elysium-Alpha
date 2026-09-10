package com.elysium.item;

import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.effect.MobEffectInstance;
import net.minecraft.world.effect.MobEffects;

import java.util.List;

public class ElysiumRuneEffects {

    /**
     * Applies real effects from socketed runes.
     * Each psionic rune type provides a distinct benefit.
     */
    public static void applyRuneEffects(Player player, ItemStack armor) {
        if (!(armor.getItem() instanceof ElysiumArmorItem)) return;
        
        ElysiumArmorItem armorItem = (ElysiumArmorItem) armor.getItem();
        List<ElysiumRuneItem.RuneType> runes = armorItem.getSocketedRunes(armor);
        
        for (ElysiumRuneItem.RuneType rune : runes) {
            switch (rune) {
                case VOIDWARD -> {
                    // Voidward: Grants Resistance when health is low
                    if (player.getHealth() < player.getMaxHealth() * 0.4f) {
                        player.addEffect(new MobEffectInstance(MobEffects.DAMAGE_RESISTANCE, 60, 0, false, false));
                    }
                }
                case PLASMAFORGE -> {
                    // Plasmaforge: Grants Strength when health is high
                    if (player.getHealth() > player.getMaxHealth() * 0.7f) {
                        player.addEffect(new MobEffectInstance(MobEffects.DAMAGE_BOOST, 40, 0, false, false));
                    }
                }
                case NEURALSPIKE -> {
                    // Neuralspike: Grants Haste (simulates faster reactions)
                    player.addEffect(new MobEffectInstance(MobEffects.DIG_SPEED, 40, 0, false, false));
                }
                case DIMENSIONALSHIFT -> {
                    // Dimensionalshift: Grants Slow Falling when falling
                    if (player.fallDistance > 2.5f) {
                        player.addEffect(new MobEffectInstance(MobEffects.SLOW_FALLING, 30, 0, false, false));
                    }
                }
                case KINETICSURGE -> {
                    // Kineticsurge: Grants Jump Boost
                    player.addEffect(new MobEffectInstance(MobEffects.JUMP, 40, 0, false, false));
                }
            }
        }
    }
}