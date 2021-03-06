/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2015 FlowKit Sarl
 * Route d'Oron 2
 * 1010 Lausanne, Switzerland
 * E-mail contact: contact@flowkit.com
 *
 * The most recent release of Palabos can be downloaded at 
 * <http://www.palabos.org/>
 *
 * The library Palabos is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** \file
 * A collection of dynamics classes (e.g. BGK) with which a Cell object
 * can be instantiated -- generic implementation.
 */
#ifndef REGULARIZED_BOUNDARY_DYNAMICS_HH
#define REGULARIZED_BOUNDARY_DYNAMICS_HH

#include "boundaryCondition/regularizedBoundaryDynamics.h"
#include "boundaryCondition/boundaryTemplates.h"
#include "core/cell.h"
#include "core/dynamicsIdentifiers.h"
#include "latticeBoltzmann/indexTemplates.h"

namespace plb {

/* *************** Class RegularizedVelocityBoundaryDynamics ************* */

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
int RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::id =
    meta::registerGeneralDynamics<T,Descriptor, RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation> >
            ( std::string("Boundary_RegularizedVelocity_")+util::val2str(direction) +
              std::string("_")+util::val2str(orientation) );

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::
    RegularizedVelocityBoundaryDynamics(Dynamics<T,Descriptor>* baseDynamics_, bool automaticPrepareCollision_)
        : VelocityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>(baseDynamics_, automaticPrepareCollision_)
{ }

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::
    RegularizedVelocityBoundaryDynamics(HierarchicUnserializer& unserializer)
        : VelocityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>(0, false)
{
    unserialize(unserializer);
}

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>*
    RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::clone() const
{
    return new RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>(*this);
}
 
template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
void RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::serialize(HierarchicSerializer& serializer) const
{
    VelocityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>::serialize(serializer);
}

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
void RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::unserialize(HierarchicUnserializer& unserializer)
{
    VelocityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>::unserialize(unserializer);
}

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
int RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::getId() const {
    return id;
}

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
void RegularizedVelocityBoundaryDynamics<T,Descriptor,direction,orientation>::
    completePopulations(Cell<T,Descriptor>& cell) const
{
    T rhoBar;
    Array<T,Descriptor<T>::d> j;
    this -> computeRhoBarJ(cell, rhoBar, j);
    T jSqr = VectorTemplate<T,Descriptor>::normSqr(j);

    Array<T,SymmetricTensor<T,Descriptor>::n> PiNeq;
    boundaryTemplates<T,Descriptor,direction,orientation>::compute_PiNeq (
            this->getBaseDynamics(), cell, rhoBar, j, jSqr, PiNeq );

    this->getBaseDynamics().regularize(cell, rhoBar, j, jSqr, PiNeq);
}


/* *************** Class RegularizedDensityBoundaryDynamics ************* */

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
int RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::id =
    meta::registerGeneralDynamics<T,Descriptor, RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation> >
            ( std::string("Boundary_RegularizedDensity_")+util::val2str(direction) +
              std::string("_")+util::val2str(orientation) );

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::
    RegularizedDensityBoundaryDynamics(Dynamics<T,Descriptor>* baseDynamics_, bool automaticPrepareCollision_)
        : DensityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>(baseDynamics_, automaticPrepareCollision_)
{ }

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::
    RegularizedDensityBoundaryDynamics(HierarchicUnserializer& unserializer)
        : DensityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>(0, false)
{
    unserialize(unserializer);
}

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>*
    RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::clone() const
{
    return new RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>(*this);
}
 
template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
int RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::getId() const {
    return id;
}

 
template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
void RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::serialize(HierarchicSerializer& serializer) const
{
    DensityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>::serialize(serializer);
}

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
void RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::unserialize(HierarchicUnserializer& unserializer)
{
    DensityDirichletBoundaryDynamics<T,Descriptor,direction,orientation>::unserialize(unserializer);
}

template<typename T, template<typename U> class Descriptor,
         int direction, int orientation>
void RegularizedDensityBoundaryDynamics<T,Descriptor,direction,orientation>::
    completePopulations(Cell<T,Descriptor>& cell) const
{
    T rhoBar;
    Array<T,Descriptor<T>::d> j;
    this -> computeRhoBarJ(cell, rhoBar, j);
    T jSqr = VectorTemplate<T,Descriptor>::normSqr(j);

    Array<T,SymmetricTensor<T,Descriptor>::n> PiNeq;
    boundaryTemplates<T,Descriptor,direction,orientation>::compute_PiNeq (
            this->getBaseDynamics(), cell, rhoBar, j, jSqr, PiNeq );

    this->getBaseDynamics().regularize(cell, rhoBar, j, jSqr, PiNeq);
}

}  // namespace plb

#endif  // REGULARIZED_BOUNDARY_DYNAMICS_HH
