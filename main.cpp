/* 
 * ola2pd - interface from Open Lighting Arquitecture to Pure Data
 * v 0.03
 *
 * Copyright (c) 2012-2013 Santiago Noreña (libremediaserver@gmail.com) 
 *
 * ola2pd is an external for Pure Data and Max that reads one DMX512 
 * universe from the Open Lighting Arquitecture and output it like a list
 * of 512 channels.  
 *
 * Based on dmxmonitor Copyright (C) 2001 Dirk Jagdmann <doj@cubic.org> 
 * and ola_dmxmonitor by Simon Newton (nomis52<AT>gmail.com) 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

// Define flext
#define FLEXT_USE_CMEM 1
//#define FLEXT_SHARED 1
#define FLEXT_ATTRIBUTES 1
#include <flext.h>
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Define general

#include <errno.h>
#include <string>

// Define ola

#include <ola/BaseTypes.h>
#include <ola/Callback.h>
#include <ola/OlaCallbackClient.h>
#include <ola/OlaClientWrapper.h>
#include <ola/DmxBuffer.h>
#include <ola/io/SelectServer.h>
#include <ola/network/TCPSocket.h>

using ola::DmxBuffer;
using ola::OlaCallbackClient;
using ola::OlaCallbackClientWrapper;
using ola::io::SelectServer;
using std::string;

class ola2pd:

	public flext_base
{

	FLEXT_HEADER_S(ola2pd,flext_base,setup)

public:
  
	// constructor with no arguments
	ola2pd(int argc,const t_atom *argv):
	// initialize data members
  m_universe(0),
  m_clientpointer(NULL),
  m_counter(0),
  m_blackout_status(0)
  //          m_stdin_descriptor(STDIN_FILENO)
  {
		// --- define inlets and outlets ---
		AddInAnything(); // default inlet
    AddOutList();	// outlet for DMX list
    AddOutList();	// outlet for individual DMX channels
    post("ola2pd v0.02 - an interface to Open Lighting Arquitecture");
    post("(C) 2012-2013 Santi Noreña libremediaserver@gmail.com");
    
    // processing command line
    if(argc > 0 && CanbeInt(argv[0])) {
      m_universe = GetAInt(argv[0]);
      post("Set universe to %i", m_universe);
      
      //m_open(); // gives trouble...
    }
    
}
	void NewDmx(unsigned int universe,
                const DmxBuffer &buffer,
                const string &error);
        void RegisterComplete(const string &error);
//      void StdinReady();
        bool CheckDataLoss();

protected:

void m_open() {
 	if (m_clientpointer == NULL) 
	{     
        // set up ola connection 
        if (!m_client.Setup()) {
            post("%s: %s",thisName(), strerror(errno));
        }
        m_clientpointer = m_client.GetClient();
        m_clientpointer->SetDmxCallback(ola::NewCallback(this, &ola2pd::NewDmx));
        m_clientpointer->RegisterUniverse(m_universe,ola::REGISTER,ola::NewSingleCallback(this, &ola2pd::RegisterComplete));
//      m_client.GetSelectServer()->AddReadDescriptor(&m_stdin_descriptor);
//      m_stdin_descriptor.SetOnData(ola::NewCallback(this, &ola2pd::StdinReady));
        m_client.GetSelectServer()->RegisterRepeatingTimeout(5000,ola::NewCallback(this, &ola2pd::CheckDataLoss));
        post("ola2pd: Init complete. Start listening...");
			  m_client.GetSelectServer()->Run();
        }    
	}

void m_close() {
    if (m_clientpointer != NULL)
		{
   		m_clientpointer->RegisterUniverse(m_universe,ola::UNREGISTER,ola::NewSingleCallback(this, &ola2pd::RegisterComplete));        
    	m_client.GetSelectServer()->Terminate();
			post("ola2pd: Close complete");		
		  m_clientpointer = NULL;	
	  }
}

void m_bang() {
    if (m_clientpointer != NULL) {post("%s listening on universe %d",thisName(),m_universe);}
		else {post("%s configured on universe %d. Send open to start listening",thisName(),m_universe);}
	}

void m_send(int address, int value) {
    if (m_clientpointer != NULL)
    {
      address = (int)std::max(address, 1);
      address = (int)std::min(address, 512);
      value = (int)std::max(value, 0);
      value = (int)std::min(value, 255);
      
      m_out_buffer.SetChannel(address-1, value);
      m_clientpointer->SendDmx(m_universe, m_out_buffer);
    }
	}
  
void m_blackout(int value) {
    if (m_clientpointer != NULL)
    {
      unsigned int size = m_out_buffer.Size();
      
      if (value && !m_blackout_status)
      {
        // copy buffer to blackout buffer
        m_blackout_buffer.Set(m_out_buffer.GetRaw(), size);
        
        // set all channels zero
        m_out_buffer.Blackout();
        m_clientpointer->SendDmx(m_universe, m_out_buffer);
        m_blackout_status = 1;
      }
      
      if (!value && m_blackout_status)
      {
        // copy buffer before blackout back
        m_out_buffer.Set(m_blackout_buffer.GetRaw(), size);
        
        m_clientpointer->SendDmx(m_universe, m_out_buffer);
        m_blackout_status = 0;
      }
    }
	}
  

// output all values from current input buffer -> eg. for storage
void m_dump() {
    
    unsigned int size = m_in_buffer.Size();
  
  post("dumping %i channels", size);
    AtomList dmxchannel;
    dmxchannel(2);
    
    // output each channel as list <channel nr> <value> instead of one big list -> by Matthias Kronlachner
    for(int z=0; z < size; z++)
    {
      SetFloat(dmxchannel[0], z+1);
      SetFloat(dmxchannel[1], m_in_buffer.Get(z));
      ToOutList(2, dmxchannel); // output each channel individually
    }
    
  }
 
  // output all values from current input buffer -> eg. for storage
  void m_get() {
    
    unsigned int size = m_out_buffer.Size();
    
    post("setting ola output buffer to pd output buffer", size);
    
    m_out_buffer.Set(m_in_buffer.GetRaw(), size);
    
  }
  
private:
    unsigned int m_universe;
    unsigned int m_counter;
    unsigned int m_blackout_status;
  
    DmxBuffer m_in_buffer;
    DmxBuffer m_out_buffer;
    DmxBuffer m_blackout_buffer;
  
//    ola::io::UnmanagedFileDescriptor m_stdin_descriptor;
    struct timeval m_last_data;
    OlaCallbackClientWrapper m_client;
		OlaCallbackClient *m_clientpointer;

  static void setup(t_classid c)
	{
	// --- set up methods (class scope) ---
	// register a bang method to the default inlet (0)
	FLEXT_CADDBANG(c,0,m_bang);
	// set up tagged methods for the default inlet (0)
	FLEXT_CADDMETHOD_(c,0,"open",m_open);
	FLEXT_CADDMETHOD_(c,0,"close",m_close);
	// --- set up attributes (class scope) ---
	FLEXT_CADDATTR_VAR1(c,"universe",m_universe);
  
  // -- send DMX data! ----
  FLEXT_CADDMETHOD_II(c,0,"send",m_send);
  // -- send DMX data! ----
  FLEXT_CADDMETHOD_I(c,0,"blackout",m_blackout);
  FLEXT_CADDMETHOD_(c,0,"dump",m_dump); // output in_buffer to right outlet
  FLEXT_CADDMETHOD_(c,0,"get",m_get); // set out_buffer to in_buffer -> take status from ola!
	}
  
	FLEXT_CALLBACK(m_bang)
	FLEXT_THREAD(m_open)
  FLEXT_CALLBACK_II(m_send)
  FLEXT_CALLBACK_I(m_blackout)
	FLEXT_CALLBACK(m_close)
  FLEXT_CALLBACK(m_dump)
  FLEXT_CALLBACK(m_get)
  
	FLEXT_ATTRVAR_I(m_universe) // wrapper functions (get and set) for integer variable universe
};
// instantiate the class (constructor takes no arguments)
FLEXT_NEW_V("ola2pd",ola2pd)

/*
 * Called when there is new DMX data
 */

void ola2pd::NewDmx(unsigned int universe,
                        const DmxBuffer &buffer,
                        const string &error) {
  m_counter++;
  gettimeofday(&m_last_data, NULL);   
  int z;
  
  AtomList dmxlist;
  dmxlist(512);
  
  AtomList dmxchannel;
  dmxchannel(2);
  
  // output each channel as list <channel nr> <value> as well -> by Matthias Kronlachner
  
  for(z=0; z < 512; z++)
  {
    SetFloat(dmxlist[z],(buffer.Get(z)));
    SetFloat(dmxchannel[0], z+1);
    SetFloat(dmxchannel[1], buffer.Get(z));
    ToOutList(1, dmxchannel); // output each channel individually
  }
  
  ToOutList(0, dmxlist); // output one list with values
  
  // copy to in_buffer
  unsigned int size = buffer.Size();
  m_in_buffer.Set(buffer.GetRaw(), size);
  
}

/*
 * Check for data loss.
 */

bool ola2pd::CheckDataLoss() {
  struct timeval now, diff;
  if (timerisset(&m_last_data)) {
    gettimeofday(&now, NULL);
    timersub(&now, &m_last_data, &diff);
    if (diff.tv_sec > 4 || (diff.tv_sec == 4 && diff.tv_usec > 4000000)) {
      // loss of data
      post("ola2pd: Timeout since last DMX input from OLA!");
    }
  }
  return true;
}

/*
 * Control de errores en el registro de Universos en OLA
 */ 
void ola2pd::RegisterComplete(const string &error) {
  if (!error.empty()) {
    post("ola2pd:Register command failed");
    m_client.GetSelectServer()->Terminate();
  }
}
