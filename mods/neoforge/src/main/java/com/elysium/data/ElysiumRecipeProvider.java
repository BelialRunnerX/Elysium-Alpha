package com.elysium.data;

import com.elysium.Elysium;
import net.minecraft.data.DataGenerator;
import net.minecraft.data.recipes.FinishedRecipe;
import net.minecraft.data.recipes.RecipeProvider;
import net.minecraft.data.recipes.ShapedRecipeBuilder;
import net.minecraft.data.recipes.ShapelessRecipeBuilder;
import net.minecraft.resources.ResourceLocation;
import net.minecraft.world.item.Items;
import net.minecraftforge.common.Tags;

import java.util.function.Consumer;

public class ElysiumRecipeProvider extends RecipeProvider {

    public ElysiumRecipeProvider(DataGenerator generator) {
        super(generator);
    }

    @Override
    protected void buildRecipes(Consumer<FinishedRecipe> consumer) {
        // Neutronium recipes
        ShapedRecipeBuilder.shaped(Elysium.NEUTRONIUM_BLOCK_ITEM.get())
            .pattern("###")
            .pattern("###")
            .pattern("###")
            .define('#', Elysium.NEUTRONIUM_INGOT.get())
            .unlockedBy("has_neutronium_ingot", has(Elysium.NEUTRONIUM_INGOT.get()))
            .save(consumer);

        ShapelessRecipeBuilder.shapeless(Elysium.NEUTRONIUM_INGOT.get(), 9)
            .requires(Elysium.NEUTRONIUM_BLOCK_ITEM.get())
            .unlockedBy("has_neutronium_block", has(Elysium.NEUTRONIUM_BLOCK_ITEM.get()))
            .save(consumer, new ResourceLocation(Elysium.MODID, "neutronium_ingot_from_block"));

        // Rune socketing (example)
        ShapelessRecipeBuilder.shapeless(Elysium.ELYSIUM_HELMET.get())
            .requires(Elysium.ELYSIUM_HELMET.get())
            .requires(Elysium.VOIDWARD_RUNE.get())
            .unlockedBy("has_voidward_rune", has(Elysium.VOIDWARD_RUNE.get()))
            .save(consumer, new ResourceLocation(Elysium.MODID, "rune_socket_voidward"));

        // Armor ascension (example)
        ShapelessRecipeBuilder.shapeless(Elysium.ELYSIUM_HELMET.get())
            .requires(Elysium.ELYSIUM_HELMET.get())
            .requires(Elysium.ELYSIUM_HELMET.get())
            .unlockedBy("has_elysium_helmet", has(Elysium.ELYSIUM_HELMET.get()))
            .save(consumer, new ResourceLocation(Elysium.MODID, "armor_ascension_helmet"));
    }
}