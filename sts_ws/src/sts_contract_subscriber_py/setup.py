from setuptools import find_packages, setup

package_name = 'sts_contract_subscriber_py'

setup(
    name=package_name,
    version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
         ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Hariharan Chandrasekaran',
    maintainer_email=(
        'hhc2306598322+hariharan-c1@users.noreply.github.com'
    ),
    description=(
        'Validates the canonical EgoState payload for M1 contract verification.'
    ),
    license='MIT',
    entry_points={
        'console_scripts': [
            'ego_state_test_subscriber = '
            'sts_contract_subscriber_py.ego_state_test_subscriber:main',
        ],
    },
)
