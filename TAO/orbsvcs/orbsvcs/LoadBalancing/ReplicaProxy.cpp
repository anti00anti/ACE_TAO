// -*- C++ -*-

// $Id$

#include "ReplicaProxy.h"
#include "LoadBalancer_i.h"

#if !defined(__ACE_INLINE__)
#include "ReplicaProxy.i"
#endif /* __ACE_INLINE__ */

ACE_RCSID(orbsvcs, ReplicaProxy, "$Id$")

ReplicaProxy_Impl::ReplicaProxy_Impl (void)
  : has_high_load_ (0),
    balancer_ (0),
    current_load_ (0),
    connected_ (0)
{
  // Nothing as yet
}

void
ReplicaProxy_Impl::current_load (CORBA::Float load,
                                 CORBA::Environment &ACE_TRY_ENV)
  ACE_THROW_SPEC ((LoadBalancing::ReplicaProxy::InvalidLoad,
                   CORBA::SystemException))
{
  // @@ Ossama: this is the point were the load dampening should
  // happen. Probably strategized....

  if (load < 0)
    ACE_THROW (LoadBalancing::ReplicaProxy::InvalidLoad ());

  this->current_load_ = load;
  // ACE_DEBUG ((LM_DEBUG, "Load[%x] = %f\n", long(this), load));

  this->balancer_->load_changed (this, ACE_TRY_ENV);
}

void
ReplicaProxy_Impl::disconnect (CORBA::Environment &ACE_TRY_ENV)
  ACE_THROW_SPEC ((LoadBalancing::ReplicaProxy::NotConnected,
                   CORBA::SystemException))
{
  // @@ Ossama: this code is not thread safe...

  if (this->connected_)
    {
      this->balancer_->disconnect (this, ACE_TRY_ENV);
      ACE_CHECK;

      this->connected_ = 0;
    }
  else
    {
      ACE_THROW (LoadBalancing::ReplicaProxy::NotConnected ());
    }
}

void ReplicaProxy_Impl::connect (LoadBalancer_Impl *balancer,
                                 LoadBalancing::ReplicaControl_ptr control,
                                 CORBA::Object_ptr replica,
                                 CORBA::Environment &ACE_TRY_ENV)
  ACE_THROW_SPEC ((LoadBalancing::ReplicaProxy::NilControl,
                   LoadBalancing::ReplicaProxy::NilReplica,
                   CORBA::SystemException))
{
  // @@ Ossama: this is a perfect example of code that is not thread
  // safe: what if we get a 'current_load' message in another thread?
  // Or a disconnect() message from a misbehaving replica? Or two
  // calls to connect?

  if (balancer == 0)
    ACE_THROW (CORBA::BAD_PARAM (
      CORBA_SystemException::_tao_minor_code (
        TAO_NULL_POINTER_MINOR_CODE,
        0),
      CORBA::COMPLETED_NO));
  else
    this->balancer_ = balancer;

  if (!CORBA::is_nil (control))
    this->control_ =  LoadBalancing::ReplicaControl::_duplicate (control);
  else
    ACE_THROW (LoadBalancing::ReplicaProxy::NilControl ());

  if (!CORBA::is_nil (control))
      this->replica_ = CORBA::Object::_duplicate (replica);
  else
    ACE_THROW (LoadBalancing::ReplicaProxy::NilReplica ());

  // Now indicate that connection to LoadBalancer has been made.
  this->connected_ = 1;
}
