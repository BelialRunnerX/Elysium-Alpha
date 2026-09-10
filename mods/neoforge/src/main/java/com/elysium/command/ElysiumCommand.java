package com.elysium.command;

import com.elysium.Elysium;
import com.elysium.character.ElysiumCharacter;
import com.elysium.character.ElysiumClass;
import com.elysium.character.ElysiumRace;
import com.elysium.network.ElysiumNetwork;
import com.elysium.standing.ElysiumStanding;
import com.elysium.stats.ElysiumStat;
import com.elysium.stats.ElysiumStats;
import com.mojang.brigadier.CommandDispatcher;
import net.minecraft.ChatFormatting;
import net.minecraft.commands.CommandSourceStack;
import net.minecraft.commands.Commands;
import net.minecraft.network.chat.Component;
import net.minecraft.world.entity.player.Player;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.neoforge.event.RegisterCommandsEvent;

/**
 * {@code /elysium} — the parts of the character system that want words rather
 * than a screen.
 *
 * <ul>
 *   <li>{@code sheet} — reopen the character screen.</li>
 *   <li>{@code stats} — print the totals into chat, which is the version you
 *       can screenshot, paste, or read on a server with the GUI scaled to
 *       something unfortunate.</li>
 *   <li>{@code standing} — Favor and Suspicion, unchanged.</li>
 *   <li>{@code respec} — hand every spent point back.</li>
 * </ul>
 *
 * Deliberately no {@code race} or {@code class} subcommand. Race is chosen
 * once and class is changed at an Ascension Forge; a command that reassigned
 * either would make the screen decorative and the forge pointless.
 */
@EventBusSubscriber(modid = Elysium.MODID)
public final class ElysiumCommand {

    private ElysiumCommand() {
    }

    @SubscribeEvent
    public static void onRegisterCommands(RegisterCommandsEvent event) {
        CommandDispatcher<CommandSourceStack> dispatcher = event.getDispatcher();

        dispatcher.register(Commands.literal("elysium")
                .then(Commands.literal("sheet").executes(context -> {
                    Player player = context.getSource().getPlayerOrException();
                    ElysiumNetwork.sendSheet(player);
                    return 1;
                }))
                .then(Commands.literal("stats").executes(context -> {
                    Player player = context.getSource().getPlayerOrException();
                    printStats(context.getSource(), player);
                    return 1;
                }))
                .then(Commands.literal("standing").executes(context -> {
                    Player player = context.getSource().getPlayerOrException();
                    context.getSource().sendSuccess(() -> ElysiumStanding.report(player), false);
                    return 1;
                }))
                .then(Commands.literal("respec").executes(context -> {
                    Player player = context.getSource().getPlayerOrException();
                    ElysiumCharacter.respec(player);
                    ElysiumNetwork.sendSheet(player);
                    context.getSource().sendSuccess(
                            () -> Component.translatable("elysium.command.respec",
                                            ElysiumCharacter.getUnspentPoints(player))
                                    .withStyle(ChatFormatting.GOLD), false);
                    return 1;
                })));
    }

    private static void printStats(CommandSourceStack source, Player player) {
        ElysiumRace race = ElysiumCharacter.getRace(player);
        ElysiumClass job = ElysiumCharacter.getElysiumClass(player);

        source.sendSuccess(() -> Component.translatable("elysium.command.header",
                        race == null ? Component.translatable("elysium.character.unchosen")
                                : race.getDisplayName(),
                        job == null ? Component.translatable("elysium.character.unchosen")
                                : job.getDisplayName(),
                        ElysiumCharacter.getLevel(player))
                .withStyle(ChatFormatting.GOLD), false);

        for (ElysiumStat stat : ElysiumStat.values()) {
            int value = ElysiumStats.get(player, stat);
            source.sendSuccess(() -> Component.empty()
                    .append(stat.getDisplayName())
                    .append(Component.literal("  " + value).withStyle(ChatFormatting.WHITE)), false);
        }
    }
}
