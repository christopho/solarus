/*
 * Copyright (C) 2006-2016 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "solarus/entities/EnemyReaction.h"
#include "solarus/lowlevel/Debug.h"
#include "solarus/Sprite.h"

namespace Solarus {

const std::string EnumInfoTraits<EnemyReaction::ReactionType>::pretty_name = "enemy reaction type";

const EnumInfo<EnemyReaction::ReactionType>::names_type EnumInfoTraits<EnemyReaction::ReactionType>::names = {
  { EnemyReaction::ReactionType::HURT, "hurt" },
  { EnemyReaction::ReactionType::IGNORED, "ignored" },
  { EnemyReaction::ReactionType::PROTECTED, "protected" },
  { EnemyReaction::ReactionType::IMMOBILIZED, "immobilized" },
  { EnemyReaction::ReactionType::CUSTOM, "custom" }
};

/**
 * \brief Constructor.
 */
EnemyReaction::EnemyReaction() {

  set_default_reaction();
}

/**
 * \brief Resets this reaction to its default value.
 */
void EnemyReaction::set_default_reaction() {

  general_reaction.type = ReactionType::IGNORED;
  general_reaction.life_lost = 0;
  sprite_reactions.clear();
}

/**
 * \brief Sets how the enemy reacts when the sprite that receives the attack
 * has no specific reaction.
 * \param reaction the reaction to set
 * \param life_lost number of life points to remove (only for reaction HURT)
 */
void EnemyReaction::set_general_reaction(ReactionType reaction, int life_lost) {

  general_reaction.type = reaction;
  if (reaction == ReactionType::HURT) {
    general_reaction.life_lost = life_lost;
  }
}

/**
 * \brief Makes the enemy reacts differently when the attack is received by a particular sprite.
 * \param sprite a sprite of the enemy (if nullptr, this is equivalent to set_general_reaction)
 * \param reaction the reaction to make when this specific sprites receives an attack
 * \param life_lost number of life points to remove (only for reaction HURT)
 */
void EnemyReaction::set_sprite_reaction(const Sprite* sprite, ReactionType reaction, int life_lost) {

  if (sprite == nullptr) {
    set_general_reaction(reaction, life_lost);
  }
  else {
    sprite_reactions[sprite].type = reaction;
    sprite_reactions[sprite].life_lost = life_lost;
  }
}

/**
 * \brief Returns the reaction to an attack on a sprite.
 * \param sprite the sprite that receives the attack
 * \return the corresponding reaction
 */
const EnemyReaction::Reaction& EnemyReaction::get_reaction(
    const Sprite* sprite) const {

  if (sprite != nullptr) {
    auto it = sprite_reactions.find(sprite);
    if (it != sprite_reactions.end()) {
      return it->second;
    }
  }
  return general_reaction;
}

}

