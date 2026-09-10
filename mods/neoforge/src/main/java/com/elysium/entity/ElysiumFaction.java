package com.elysium.entity;

import net.minecraft.world.entity.Entity;
import net.minecraft.world.entity.monster.Monster;

/**
 * Which side of the Empire a mob is on.
 *
 * The world is not divided into "vanilla" and "modded" — it is divided into
 * the Empire and everyone else. Ordinary hostiles are {@link #UNSWORN}, because
 * from the Empire's point of view a creeper is exactly as unsanctioned as a
 * rebel: it is simply something out there that has not sworn.
 *
 * That matters mechanically. If only the mod's own mobs counted, a player would
 * have to hunt Elysium content to move either meter, and the standing system
 * would sit in a corner of the game rather than running through all of it.
 * Passive animals stay {@link #NEUTRAL} — killing a cow is not service.
 */
public enum ElysiumFaction {

    /** The Empire's own. Killing these raises Suspicion. */
    EMPIRE,

    /** Everything hostile that is not the Empire's. Killing these raises Favor. */
    UNSWORN,

    /** Everything else. Moves neither meter. */
    NEUTRAL;

    public static ElysiumFaction of(Entity entity) {
        // Order matters: both custom mobs extend Monster further up the chain,
        // so the specific checks have to come first.
        if (entity instanceof ImperialEnforcer) {
            return EMPIRE;
        }
        if (entity instanceof UnswornRaider) {
            return UNSWORN;
        }
        if (entity instanceof Monster) {
            return UNSWORN;
        }
        return NEUTRAL;
    }

    /** True for the mod's own faction mobs, which always pay out. */
    public static boolean isNamedCombatant(Entity entity) {
        return entity instanceof ImperialEnforcer || entity instanceof UnswornRaider;
    }
}
