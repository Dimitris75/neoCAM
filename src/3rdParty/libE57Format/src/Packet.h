/*
 * Original work Copyright 2009 - 2010 Kevin Ackley (kackley@gwi.net)
 * Modified work Copyright 2018 - 2020 Andy Maloney <asmaloney@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <vector>

#include "Common.h"

namespace e57
{
   class CheckedFile;
   class PacketLock;

   /// Packet types (in a compressed vector section)
   enum
   {
      INDEX_PACKET = 0,
      DATA_PACKET,
      EMPTY_PACKET,
   };

   /// maximum size of CompressedVector binary data packet
   constexpr int DATA_PACKET_MAX = ( 64 * 1024 );

   class PacketReadCache
   {
   public:
      PacketReadCache( CheckedFile *cFile, unsigned packetCount );

      std::unique_ptr<PacketLock> lock( uint64_t packetLogicalOffset,
                                        char *&pkt ); //??? pkt could be const

#ifdef E57_DEBUG
      void dump( int indent = 0, std::ostream &os = std::cout );
#endif

   protected:
      /// Only PacketLock can unlock the cache
      friend class PacketLock;
      void unlock( unsigned cacheIndex );

      void readPacket( unsigned oldestEntry, uint64_t packetLogicalOffset );

      struct CacheEntry
      {
         uint64_t logicalOffset_ = 0;
         char buffer_[DATA_PACKET_MAX]; //! No need to init since it's a data buffer
         unsigned lastUsed_ = 0;
      };

      unsigned lockCount_ = 0;
      unsigned useCount_ = 0;
      CheckedFile *cFile_ = nullptr;

      std::vector<CacheEntry> entries_;
   };

   class PacketLock
   {
   public:
      ~PacketLock();

   private:
      /// Can't be copied or assigned
      PacketLock( const PacketLock &plock );
      PacketLock &operator=( const PacketLock &plock );

   protected:
      friend class PacketReadCache;
      /// Only PacketReadCache can construct
      PacketLock( PacketReadCache *cache, unsigned cacheIndex );

      PacketReadCache *cache_ = nullptr;
      unsigned int cacheIndex_ = 0;
   };

   class DataPacketHeader
   {
   public:
      DataPacketHeader();

      void reset();

      void verify( unsigned bufferLength = 0 ) const; //???use

#ifdef E57_DEBUG
      void dump( int indent = 0, std::ostream &os = std::cout ) const;
#endif
      const uint8_t packetType = DATA_PACKET;

      uint8_t packetFlags = 0;
      uint16_t packetLogicalLengthMinus1 = 0;
      uint16_t bytestreamCount = 0;
   };

   class DataPacket
   {
   public:
      DataPacket();

      void verify( unsigned bufferLength = 0 ) const;
      char *getBytestream( unsigned bytestreamNumber, unsigned &byteCount );
      unsigned getBytestreamBufferLength( unsigned bytestreamNumber );

#ifdef E57_DEBUG
      void dump( int indent = 0, std::ostream &os = std::cout ) const;
#endif

      static constexpr int PayloadSize = DATA_PACKET_MAX - sizeof( DataPacketHeader );

      DataPacketHeader header;

      uint8_t payload[PayloadSize]; //! No need to init since it's a data buffer
   };
}
