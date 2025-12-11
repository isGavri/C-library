0000000000001000 <syscall1>:
    1000:	55                   	push   %rbp
    1001:	48 89 e5             	mov    %rsp,%rbp
    1004:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    1008:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
    100c:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    1010:	48 8b 55 e0          	mov    -0x20(%rbp),%rdx
    1014:	48 89 d7             	mov    %rdx,%rdi
    1017:	0f 05                	syscall
    1019:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    101d:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    1021:	5d                   	pop    %rbp
    1022:	c3                   	ret

0000000000001023 <syscall2>:
    1023:	55                   	push   %rbp
    1024:	48 89 e5             	mov    %rsp,%rbp
    1027:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    102b:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
    102f:	48 89 55 d8          	mov    %rdx,-0x28(%rbp)
    1033:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    1037:	48 8b 55 e0          	mov    -0x20(%rbp),%rdx
    103b:	48 8b 75 d8          	mov    -0x28(%rbp),%rsi
    103f:	48 89 d7             	mov    %rdx,%rdi
    1042:	0f 05                	syscall
    1044:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    1048:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    104c:	5d                   	pop    %rbp
    104d:	c3                   	ret

000000000000104e <syscall3>:
    104e:	55                   	push   %rbp
    104f:	48 89 e5             	mov    %rsp,%rbp
    1052:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    1056:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
    105a:	48 89 55 d8          	mov    %rdx,-0x28(%rbp)
    105e:	48 89 4d d0          	mov    %rcx,-0x30(%rbp)
    1062:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    1066:	48 8b 7d e0          	mov    -0x20(%rbp),%rdi
    106a:	48 8b 75 d8          	mov    -0x28(%rbp),%rsi
    106e:	48 8b 55 d0          	mov    -0x30(%rbp),%rdx
    1072:	0f 05                	syscall
    1074:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    1078:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    107c:	5d                   	pop    %rbp
    107d:	c3                   	ret

000000000000107e <parse_int>:
    107e:	55                   	push   %rbp
    107f:	48 89 e5             	mov    %rsp,%rbp
    1082:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    1086:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    108a:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
    108e:	eb 25                	jmp    10b5 <parse_int+0x37>
    1090:	8b 55 fc             	mov    -0x4(%rbp),%edx
    1093:	89 d0                	mov    %edx,%eax
    1095:	c1 e0 02             	shl    $0x2,%eax
    1098:	01 d0                	add    %edx,%eax
    109a:	01 c0                	add    %eax,%eax
    109c:	89 c2                	mov    %eax,%edx
    109e:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    10a2:	0f b6 00             	movzbl (%rax),%eax
    10a5:	0f be c0             	movsbl %al,%eax
    10a8:	83 e8 30             	sub    $0x30,%eax
    10ab:	01 d0                	add    %edx,%eax
    10ad:	89 45 fc             	mov    %eax,-0x4(%rbp)
    10b0:	48 83 45 f0 01       	addq   $0x1,-0x10(%rbp)
    10b5:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    10b9:	0f b6 00             	movzbl (%rax),%eax
    10bc:	3c 2f                	cmp    $0x2f,%al
    10be:	7e 0b                	jle    10cb <parse_int+0x4d>
    10c0:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    10c4:	0f b6 00             	movzbl (%rax),%eax
    10c7:	3c 39                	cmp    $0x39,%al
    10c9:	7e c5                	jle    1090 <parse_int+0x12>
    10cb:	8b 45 fc             	mov    -0x4(%rbp),%eax
    10ce:	5d                   	pop    %rbp
    10cf:	c3                   	ret

00000000000010d0 <strlen>:
    10d0:	55                   	push   %rbp
    10d1:	48 89 e5             	mov    %rsp,%rbp
    10d4:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    10d8:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    10dc:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    10e0:	eb 05                	jmp    10e7 <strlen+0x17>
    10e2:	48 83 45 f8 01       	addq   $0x1,-0x8(%rbp)
    10e7:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    10eb:	0f b6 00             	movzbl (%rax),%eax
    10ee:	84 c0                	test   %al,%al
    10f0:	75 f0                	jne    10e2 <strlen+0x12>
    10f2:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    10f6:	48 2b 45 e8          	sub    -0x18(%rbp),%rax
    10fa:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
    10fe:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    1102:	5d                   	pop    %rbp
    1103:	c3                   	ret

0000000000001104 <sleep>:
    1104:	55                   	push   %rbp
    1105:	48 89 e5             	mov    %rsp,%rbp
    1108:	48 83 ec 20          	sub    $0x20,%rsp
    110c:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    1110:	48 c7 45 f0 00 00 00 	movq   $0x0,-0x10(%rbp)
    1117:	00 
    1118:	48 c7 45 f8 00 00 00 	movq   $0x0,-0x8(%rbp)
    111f:	00 
    1120:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    1124:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
    1128:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
    112c:	ba 00 00 00 00       	mov    $0x0,%edx
    1131:	48 89 c6             	mov    %rax,%rsi
    1134:	bf 23 00 00 00       	mov    $0x23,%edi
    1139:	e8 e5 fe ff ff       	call   1023 <syscall2>
    113e:	90                   	nop
    113f:	c9                   	leave
    1140:	c3                   	ret

0000000000001141 <print>:
    1141:	55                   	push   %rbp
    1142:	48 89 e5             	mov    %rsp,%rbp
    1145:	48 83 ec 08          	sub    $0x8,%rsp
    1149:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
    114d:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    1151:	48 89 c7             	mov    %rax,%rdi
    1154:	e8 77 ff ff ff       	call   10d0 <strlen>
    1159:	48 89 c2             	mov    %rax,%rdx
    115c:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    1160:	48 89 d1             	mov    %rdx,%rcx
    1163:	48 89 c2             	mov    %rax,%rdx
    1166:	be 01 00 00 00       	mov    $0x1,%esi
    116b:	bf 01 00 00 00       	mov    $0x1,%edi
    1170:	e8 d9 fe ff ff       	call   104e <syscall3>
    1175:	90                   	nop
    1176:	c9                   	leave
    1177:	c3                   	ret

0000000000001178 <main>:
    1178:	55                   	push   %rbp
    1179:	48 89 e5             	mov    %rsp,%rbp
    117c:	48 83 ec 20          	sub    $0x20,%rsp
    1180:	89 7d ec             	mov    %edi,-0x14(%rbp)
    1183:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
    1187:	83 7d ec 02          	cmpl   $0x2,-0x14(%rbp)
    118b:	74 16                	je     11a3 <main+0x2b>
    118d:	48 8d 05 6c 0e 00 00 	lea    0xe6c(%rip),%rax        # 2000 <_start+0xde3>
    1194:	48 89 c7             	mov    %rax,%rdi
    1197:	e8 a5 ff ff ff       	call   1141 <print>
    119c:	b8 01 00 00 00       	mov    $0x1,%eax
    11a1:	eb 59                	jmp    11fc <main+0x84>
    11a3:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    11a7:	48 8b 40 08          	mov    0x8(%rax),%rax
    11ab:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    11af:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    11b3:	48 89 c7             	mov    %rax,%rdi
    11b6:	e8 c3 fe ff ff       	call   107e <parse_int>
    11bb:	48 98                	cltq
    11bd:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
    11c1:	48 8d 05 66 0e 00 00 	lea    0xe66(%rip),%rax        # 202e <_start+0xe11>
    11c8:	48 89 c7             	mov    %rax,%rdi
    11cb:	e8 71 ff ff ff       	call   1141 <print>
    11d0:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    11d4:	48 89 c7             	mov    %rax,%rdi
    11d7:	e8 65 ff ff ff       	call   1141 <print>
    11dc:	48 8d 05 59 0e 00 00 	lea    0xe59(%rip),%rax        # 203c <_start+0xe1f>
    11e3:	48 89 c7             	mov    %rax,%rdi
    11e6:	e8 56 ff ff ff       	call   1141 <print>
    11eb:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    11ef:	48 89 c7             	mov    %rax,%rdi
    11f2:	e8 0d ff ff ff       	call   1104 <sleep>
    11f7:	b8 00 00 00 00       	mov    $0x0,%eax
    11fc:	c9                   	leave
    11fd:	c3                   	ret

00000000000011fe <exit>:
    11fe:	55                   	push   %rbp
    11ff:	48 89 e5             	mov    %rsp,%rbp
    1202:	48 83 ec 08          	sub    $0x8,%rsp
    1206:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
    120a:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    120e:	48 89 c6             	mov    %rax,%rsi
    1211:	bf 3c 00 00 00       	mov    $0x3c,%edi
    1216:	e8 e5 fd ff ff       	call   1000 <syscall1>
    121b:	eb fe                	jmp    121b <exit+0x1d>

000000000000121d <_start>:
    121d:	31 ed                	xor    %ebp,%ebp
    121f:	48 8b 3c 24          	mov    (%rsp),%rdi
    1223:	48 8d 74 24 08       	lea    0x8(%rsp),%rsi
    1228:	48 83 e4 f0          	and    $0xfffffffffffffff0,%rsp
    122c:	e8 47 ff ff ff       	call   1178 <main>
    1231:	48 89 c7             	mov    %rax,%rdi
    1234:	e8 c5 ff ff ff       	call   11fe <exit>
    1239:	90                   	nop
    123a:	0f 0b                	ud2
