package com.elysium.item;

import net.minecraft.world.item.Item;
import net.minecraft.world.item.ItemStack;

public class ElysiumRuneItem extends Item {

    private final RuneType type;

    public ElysiumRuneItem(RuneType type, Properties properties) {
        super(properties);
        this.type = type;
    }

    public RuneType getRuneType() {
        return type;
    }

    public enum RuneType {
        VOIDWARD("voidward", "Void Resistance"),
        PLASMAFORGE("plasmaforge", "Plasma Damage"),
        NEURALSPIKE("neuralspike", "Neural Disruption"),
        DIMENSIONALSHIFT("dimensionalshift", "Dimensional Mobility"),
        KINETICSURGE("kineticsurge", "Kinetic Force");

        private final String id;
        private final String effect;

        RuneType(String id, String effect) {
            this.id = id;
            this.effect = effect;
        }

        public String getId() {
            return id;
        }

        public String getEffect() {
            return effect;
        }
    }
}