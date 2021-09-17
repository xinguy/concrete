// RUN: zamacompiler --entry-dialect=lowlfhe --action=roundtrip %s 2>&1| FileCheck %s

// CHECK-LABEL: func @add_lwe_ciphertexts(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7>
func @add_lwe_ciphertexts(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.add_lwe_ciphertexts"(%arg0, %arg1) : (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.lwe_ciphertext<2048,7>

  %1 = "LowLFHE.add_lwe_ciphertexts"(%arg0, %arg1): (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.lwe_ciphertext<2048,7>) -> (!LowLFHE.lwe_ciphertext<2048,7>)
  return %1: !LowLFHE.lwe_ciphertext<2048,7>
}

// CHECK-LABEL: func @add_plaintext_lwe_ciphertext(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.plaintext<5>) -> !LowLFHE.lwe_ciphertext<2048,7>
func @add_plaintext_lwe_ciphertext(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.plaintext<5>) -> !LowLFHE.lwe_ciphertext<2048,7> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.add_plaintext_lwe_ciphertext"(%arg0, %arg1) : (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.plaintext<5>) -> !LowLFHE.lwe_ciphertext<2048,7>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.lwe_ciphertext<2048,7>

  %1 = "LowLFHE.add_plaintext_lwe_ciphertext"(%arg0, %arg1): (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.plaintext<5>) -> (!LowLFHE.lwe_ciphertext<2048,7>)
  return %1: !LowLFHE.lwe_ciphertext<2048,7>
}

// CHECK-LABEL: func @mul_cleartext_lwe_ciphertext(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.cleartext<7>) -> !LowLFHE.lwe_ciphertext<2048,7>
func @mul_cleartext_lwe_ciphertext(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.cleartext<7>) -> !LowLFHE.lwe_ciphertext<2048,7> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.mul_cleartext_lwe_ciphertext"(%arg0, %arg1) : (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.cleartext<7>) -> !LowLFHE.lwe_ciphertext<2048,7>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.lwe_ciphertext<2048,7>

  %1 = "LowLFHE.mul_cleartext_lwe_ciphertext"(%arg0, %arg1): (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.cleartext<7>) -> (!LowLFHE.lwe_ciphertext<2048,7>)
  return %1: !LowLFHE.lwe_ciphertext<2048,7>
}

// CHECK-LABEL: func @negate_lwe_ciphertext(%arg0: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7>
func @negate_lwe_ciphertext(%arg0: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.negate_lwe_ciphertext"(%arg0) : (!LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.lwe_ciphertext<2048,7>

  %1 = "LowLFHE.negate_lwe_ciphertext"(%arg0): (!LowLFHE.lwe_ciphertext<2048,7>) -> (!LowLFHE.lwe_ciphertext<2048,7>)
  return %1: !LowLFHE.lwe_ciphertext<2048,7>
}

// CHECK-LABEL: func @bootstrap_lwe(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.glwe_ciphertext) -> !LowLFHE.lwe_ciphertext<2048,7>
func @bootstrap_lwe(%arg0: !LowLFHE.lwe_ciphertext<2048,7>, %arg1: !LowLFHE.glwe_ciphertext) -> !LowLFHE.lwe_ciphertext<2048,7> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.bootstrap_lwe"(%arg0, %arg1) {baseLog = -1 : i32, k = 1 : i32, level = -1 : i32, polynomialSize = 1024 : i32} : (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.glwe_ciphertext) -> !LowLFHE.lwe_ciphertext<2048,7>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.lwe_ciphertext<2048,7>

  %1 = "LowLFHE.bootstrap_lwe"(%arg0, %arg1) {baseLog = -1 : i32, k = 1 : i32, level = -1 : i32, polynomialSize = 1024 : i32} : (!LowLFHE.lwe_ciphertext<2048,7>, !LowLFHE.glwe_ciphertext) -> (!LowLFHE.lwe_ciphertext<2048,7>)
  return %1: !LowLFHE.lwe_ciphertext<2048,7>
}

// CHECK-LABEL: func @decrypt_glwe(%arg0: !LowLFHE.glwe_secret_key, %arg1: !LowLFHE.glwe_ciphertext) -> !LowLFHE.plaintext_list
func @decrypt_glwe(%arg0: !LowLFHE.glwe_secret_key, %arg1: !LowLFHE.glwe_ciphertext) -> !LowLFHE.plaintext_list {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.decrypt_glwe"(%arg0, %arg1) : (!LowLFHE.glwe_secret_key, !LowLFHE.glwe_ciphertext) -> !LowLFHE.plaintext_list
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.plaintext_list

  %1 = "LowLFHE.decrypt_glwe"(%arg0, %arg1): (!LowLFHE.glwe_secret_key, !LowLFHE.glwe_ciphertext) -> (!LowLFHE.plaintext_list)
  return %1: !LowLFHE.plaintext_list
}

// CHECK-LABEL: func @decrypt_lwe(%arg0: !LowLFHE.lwe_secret_key, %arg1: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.plaintext<6>
func @decrypt_lwe(%arg0: !LowLFHE.lwe_secret_key, %arg1: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.plaintext<6> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.decrypt_lwe"(%arg0, %arg1) : (!LowLFHE.lwe_secret_key, !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.plaintext<6>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.plaintext<6>

  %1 = "LowLFHE.decrypt_lwe"(%arg0, %arg1): (!LowLFHE.lwe_secret_key, !LowLFHE.lwe_ciphertext<2048,7>) -> (!LowLFHE.plaintext<6>)
  return %1: !LowLFHE.plaintext<6>
}

// CHECK-LABEL: func @encrypt_glwe(%arg0: !LowLFHE.glwe_secret_key, %arg1: !LowLFHE.plaintext_list, %arg2: !LowLFHE.enc_rand_gen, %arg3: !LowLFHE.variance) -> !LowLFHE.glwe_ciphertext
func @encrypt_glwe(%arg0: !LowLFHE.glwe_secret_key, %arg1: !LowLFHE.plaintext_list, %arg2: !LowLFHE.enc_rand_gen, %arg3: !LowLFHE.variance) -> !LowLFHE.glwe_ciphertext {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.encrypt_glwe"(%arg0, %arg1, %arg2, %arg3) : (!LowLFHE.glwe_secret_key, !LowLFHE.plaintext_list, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> !LowLFHE.glwe_ciphertext
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.glwe_ciphertext

  %1 = "LowLFHE.encrypt_glwe"(%arg0, %arg1, %arg2, %arg3): (!LowLFHE.glwe_secret_key, !LowLFHE.plaintext_list, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> (!LowLFHE.glwe_ciphertext)
  return %1: !LowLFHE.glwe_ciphertext
}

// CHECK-LABEL: func @encrypt_lwe(%arg0: !LowLFHE.lwe_secret_key, %arg1: !LowLFHE.plaintext<6>, %arg2: !LowLFHE.enc_rand_gen, %arg3: !LowLFHE.variance) -> !LowLFHE.lwe_ciphertext<2048,7>
func @encrypt_lwe(%arg0: !LowLFHE.lwe_secret_key, %arg1: !LowLFHE.plaintext<6>, %arg2: !LowLFHE.enc_rand_gen, %arg3: !LowLFHE.variance) -> !LowLFHE.lwe_ciphertext<2048,7> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.encrypt_lwe"(%arg0, %arg1, %arg2, %arg3) : (!LowLFHE.lwe_secret_key, !LowLFHE.plaintext<6>, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> !LowLFHE.lwe_ciphertext<2048,7>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.lwe_ciphertext<2048,7>

  %1 = "LowLFHE.encrypt_lwe"(%arg0, %arg1, %arg2, %arg3): (!LowLFHE.lwe_secret_key, !LowLFHE.plaintext<6>, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> (!LowLFHE.lwe_ciphertext<2048,7>)
  return %1: !LowLFHE.lwe_ciphertext<2048,7>
}

// CHECK-LABEL: func @get_plaintext_list_element(%arg0: !LowLFHE.plaintext_list, %arg1: index) -> i7
func @get_plaintext_list_element(%arg0: !LowLFHE.plaintext_list, %arg1: index) -> i7 {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.get_plaintext_list_element"(%arg0, %arg1) : (!LowLFHE.plaintext_list, index) -> i7
  // CHECK-NEXT: return %[[V1]] : i7

  %1 = "LowLFHE.get_plaintext_list_element"(%arg0, %arg1): (!LowLFHE.plaintext_list, index) -> i7
  return %1: i7
}

// CHECK-LABEL: func @set_plaintext_list_element(%arg0: !LowLFHE.plaintext_list, %arg1: index, %arg2: i7)
func @set_plaintext_list_element(%arg0: !LowLFHE.plaintext_list, %arg1: index, %arg2: i7){
  // CHECK-NEXT: "LowLFHE.set_plaintext_list_element"(%arg0, %arg1, %arg2) : (!LowLFHE.plaintext_list, index, i7) -> ()
  // CHECK-NEXT: return

  "LowLFHE.set_plaintext_list_element"(%arg0, %arg1, %arg2): (!LowLFHE.plaintext_list, index, i7) -> ()
  return
}

// CHECK-LABEL: func @keyswitch_lwe(%arg0: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7>
func @keyswitch_lwe(%arg0: !LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7> {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.keyswitch_lwe"(%arg0) {baseLog = 2 : i32, inputLweSize = 1 : i32, level = 3 : i32, outputLweSize = 1 : i32} : (!LowLFHE.lwe_ciphertext<2048,7>) -> !LowLFHE.lwe_ciphertext<2048,7>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.lwe_ciphertext<2048,7>

  %1 = "LowLFHE.keyswitch_lwe"(%arg0){baseLog = 2 : i32, inputLweSize = 1 : i32, level = 3 : i32, outputLweSize = 1 : i32}: (!LowLFHE.lwe_ciphertext<2048,7>) -> (!LowLFHE.lwe_ciphertext<2048,7>)
  return %1: !LowLFHE.lwe_ciphertext<2048,7>
}

// CHECK-LABEL: func @fill_glwe_secret_key(%arg0: !LowLFHE.glwe_secret_key, %arg1: !LowLFHE.secret_rand_gen)
func @fill_glwe_secret_key(%arg0: !LowLFHE.glwe_secret_key, %arg1: !LowLFHE.secret_rand_gen) {
  // CHECK-NEXT: "LowLFHE.fill_glwe_secret_key"(%arg0, %arg1) : (!LowLFHE.glwe_secret_key, !LowLFHE.secret_rand_gen) -> ()
  // CHECK-NEXT: return

  "LowLFHE.fill_glwe_secret_key"(%arg0, %arg1): (!LowLFHE.glwe_secret_key, !LowLFHE.secret_rand_gen) -> ()
  return
}

// CHECK-LABEL: func @fill_lwe_secret_key(%arg0: !LowLFHE.lwe_secret_key, %arg1: !LowLFHE.secret_rand_gen)
func @fill_lwe_secret_key(%arg0: !LowLFHE.lwe_secret_key, %arg1: !LowLFHE.secret_rand_gen) {
  // CHECK-NEXT: "LowLFHE.fill_lwe_secret_key"(%arg0, %arg1) : (!LowLFHE.lwe_secret_key, !LowLFHE.secret_rand_gen) -> ()
  // CHECK-NEXT: return

  "LowLFHE.fill_lwe_secret_key"(%arg0, %arg1): (!LowLFHE.lwe_secret_key, !LowLFHE.secret_rand_gen) -> ()
  return
}

// CHECK-LABEL: func @fill_lwe_bootstrap_key(%arg0: !LowLFHE.lwe_bootstrap_key, %arg1: !LowLFHE.lwe_secret_key, %arg2: !LowLFHE.glwe_secret_key, %arg3: !LowLFHE.enc_rand_gen, %arg4: !LowLFHE.variance)
func @fill_lwe_bootstrap_key(%arg0: !LowLFHE.lwe_bootstrap_key, %arg1: !LowLFHE.lwe_secret_key, %arg2: !LowLFHE.glwe_secret_key, %arg3: !LowLFHE.enc_rand_gen, %arg4: !LowLFHE.variance) {
  // CHECK-NEXT: "LowLFHE.fill_lwe_bootstrap_key"(%arg0, %arg1, %arg2, %arg3, %arg4) : (!LowLFHE.lwe_bootstrap_key, !LowLFHE.lwe_secret_key, !LowLFHE.glwe_secret_key, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> ()
  // CHECK-NEXT: return

  "LowLFHE.fill_lwe_bootstrap_key"(%arg0, %arg1, %arg2, %arg3, %arg4): (!LowLFHE.lwe_bootstrap_key, !LowLFHE.lwe_secret_key, !LowLFHE.glwe_secret_key, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> ()
  return
}

// CHECK-LABEL: func @fill_lwe_keyswitch_key(%arg0: !LowLFHE.lwe_key_switch_key, %arg1: !LowLFHE.lwe_secret_key, %arg2: !LowLFHE.lwe_secret_key, %arg3: !LowLFHE.enc_rand_gen, %arg4: !LowLFHE.variance)
func @fill_lwe_keyswitch_key(%arg0: !LowLFHE.lwe_key_switch_key, %arg1: !LowLFHE.lwe_secret_key, %arg2: !LowLFHE.lwe_secret_key, %arg3: !LowLFHE.enc_rand_gen, %arg4: !LowLFHE.variance) {
  // CHECK-NEXT: "LowLFHE.fill_lwe_keyswitch_key"(%arg0, %arg1, %arg2, %arg3, %arg4) : (!LowLFHE.lwe_key_switch_key, !LowLFHE.lwe_secret_key, !LowLFHE.lwe_secret_key, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> ()
  // CHECK-NEXT: return

  "LowLFHE.fill_lwe_keyswitch_key"(%arg0, %arg1, %arg2, %arg3, %arg4): (!LowLFHE.lwe_key_switch_key, !LowLFHE.lwe_secret_key, !LowLFHE.lwe_secret_key, !LowLFHE.enc_rand_gen, !LowLFHE.variance) -> ()
  return
}

// CHECK-LABEL: func @fill_plaintext_list_with_expansion(%arg0: !LowLFHE.plaintext_list, %arg1: !LowLFHE.foreign_plaintext_list)
func @fill_plaintext_list_with_expansion(%arg0: !LowLFHE.plaintext_list, %arg1: !LowLFHE.foreign_plaintext_list) {
  // CHECK-NEXT: "LowLFHE.fill_plaintext_list_with_expansion"(%arg0, %arg1) : (!LowLFHE.plaintext_list, !LowLFHE.foreign_plaintext_list) -> ()
  // CHECK-NEXT: return

  "LowLFHE.fill_plaintext_list_with_expansion"(%arg0, %arg1): (!LowLFHE.plaintext_list, !LowLFHE.foreign_plaintext_list) -> ()
  return
}

// CHECK-LABEL: func @encode_cleartext(%arg0: !LowLFHE.cleartext<6>) -> !LowLFHE.plaintext<6>
func @encode_cleartext(%arg0: !LowLFHE.cleartext<6>) -> (!LowLFHE.plaintext<6>) {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.encode_cleartext"(%arg0) : (!LowLFHE.cleartext<6>) -> !LowLFHE.plaintext<6>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.plaintext<6>

  %0 = "LowLFHE.encode_cleartext"(%arg0): (!LowLFHE.cleartext<6>) -> !LowLFHE.plaintext<6>
  return %0: !LowLFHE.plaintext<6>
}

// CHECK-LABEL: func @encode_int(%arg0: i6) -> !LowLFHE.plaintext<6>
func @encode_int(%arg0: i6) -> (!LowLFHE.plaintext<6>) {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.encode_int"(%arg0) : (i6) -> !LowLFHE.plaintext<6>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.plaintext<6>

  %0 = "LowLFHE.encode_int"(%arg0): (i6) -> !LowLFHE.plaintext<6>
  return %0: !LowLFHE.plaintext<6>
}

// CHECK-LABEL: func @const_cleartext() -> !LowLFHE.cleartext<8>
func @const_cleartext() -> (!LowLFHE.cleartext<8>) {
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.const_cleartext"() {value = 1 : i8} : () -> !LowLFHE.cleartext<8>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.cleartext<8>
  %0 = "LowLFHE.const_cleartext"() {value = 1 : i8} : () -> !LowLFHE.cleartext<8>
  return %0: !LowLFHE.cleartext<8>
}

// CHECK-LABEL: func @int_to_cleartext() -> !LowLFHE.cleartext<6>
func @int_to_cleartext() -> !LowLFHE.cleartext<6> {
  // CHECK-NEXT: %[[V0:.*]] = constant 5 : i6
  // CHECK-NEXT: %[[V1:.*]] = "LowLFHE.int_to_cleartext"(%[[V0]]) : (i6) -> !LowLFHE.cleartext<6>
  // CHECK-NEXT: return %[[V1]] : !LowLFHE.cleartext<6>
  %0 = constant 5 : i6
  %1 = "LowLFHE.int_to_cleartext"(%0) : (i6) -> !LowLFHE.cleartext<6>
  return %1 : !LowLFHE.cleartext<6>
}
