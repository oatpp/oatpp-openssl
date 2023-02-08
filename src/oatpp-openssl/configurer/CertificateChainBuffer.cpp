/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "CertificateChainBuffer.hpp"

namespace oatpp { namespace openssl { namespace configurer {

CertificateChainBuffer::CertificateChainBuffer(void *certificateChainBuffer, int certificateChainBufferLength)
{
  auto buffer = std::shared_ptr<BIO>(BIO_new_mem_buf(certificateChainBuffer, certificateChainBufferLength), BIO_free);
  m_chainOfCertificates = std::shared_ptr<X509>(PEM_read_bio_X509(buffer.get(), nullptr, nullptr, nullptr), X509_free);
  if (m_chainOfCertificates == nullptr) {
    throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::CertificateChainBuffer()]: Error. "
                             "'m_chainOfCertificates' == nullptr.");
  }
}

void CertificateChainBuffer::configure(SSL_CTX *ctx) {
  if (SSL_CTX_add0_chain_cert(ctx, m_chainOfCertificates.get()) <= 0) {
    throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::configure()]: Error. "
                             "Call to 'SSL_CTX_set1_chain' failed.");
  }
}

}}}
