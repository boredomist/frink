/*   This file is part of Frink.
 *
 *   Frink is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Frink is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Frink.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "block.h"
#include "loader.h"

AttoBlock* compileFrink(FrinkProgram*);
TValue Token_to_TValue(Token);
#endif /* _COMPILER_H_ */
